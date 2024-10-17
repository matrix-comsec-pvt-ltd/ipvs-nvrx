#include "CameraListButton.h"
#include "ApplController.h"
#include "EnumFile.h"
#include <QPaintEvent>
#include <QPainter>

#define CAMERA_LIST_BUTTON_WIDTH_ADDCAM        SCALE_WIDTH(310)

#define CAMERA_LIST_BUTTON_WIDTH               SCALE_WIDTH(250)
#define CAMERA_LIST_BUTTON_HEIGHT              SCALE_HEIGHT(30)

#define CAMERA_LIST_ICON_PATH        ":/Images_Nvrx/CameraListIcon/"
#define CAMERA_LIST_IMG_ICON_SOURCE  CAMERA_LIST_ICON_PATH"Camera/"

const QString cameraImgPath[] = {"Normal.png",
                                 "Connecting_Assigned.png",
                                 "Connected_Assigned.png",
                                 "Retry_Assigned.png",
                                 "Assigned.png"};

const QString connectAllStr[] = {"ConnectAll.png",
                                 "DisconnectAll.png",
                                 "DisconnectAll.png",
                                 "DisconnectAll.png",
                                 "DisconnectAll.png"};

CameraListButton::CameraListButton(int index,
                                   QString label,
                                   CAMERA_STATE_TYPE_e connectionState,
                                   QWidget *parent,
                                   int indexInPage,
                                   bool isEnabled, int deviceIndex,
                                   CAMERALIST_CALLED_BY_e createdBy)
    : MenuButton(index,
          ((createdBy == CALLED_BY_VIEWCAM_ADD_LIST)? CAMERA_LIST_BUTTON_WIDTH_ADDCAM:CAMERA_LIST_BUTTON_WIDTH),
          CAMERA_LIST_BUTTON_HEIGHT,
          label,
          parent,
          SCALE_WIDTH(60), 0, 0,
          indexInPage,
          isEnabled,
          isEnabled,
          false,
          false,
          true,
          true,
          NORMAL_FONT_COLOR,
          deviceIndex), m_currentConnectionState(MAX_CAMERA_STATE), m_connectStr(CONNECT_ALL_STR)
{
    m_createdBy  = createdBy;

    updateConnectionState(connectionState);
}

void CameraListButton::updateConnectionState(CAMERA_STATE_TYPE_e connectionState)
{
    if(m_currentConnectionState != connectionState)
    {
        m_currentConnectionState = connectionState;
        changeImage();
    }
}

void CameraListButton::changeImage()
{
    if((m_index != 0) || (m_createdBy == CALLED_BY_VIEWCAM_ADD_LIST) )
    {
        m_imageSource = QString(CAMERA_LIST_IMG_ICON_SOURCE) + cameraImgPath[m_currentConnectionState];
    }
    else
    {
        m_imageSource = QString(CAMERA_LIST_ICON_PATH) + connectAllStr[m_currentConnectionState];
    }
    m_iconImage = QPixmap(m_imageSource);
    SCALE_IMAGE(m_iconImage);
    update ();
}

void CameraListButton::changeText(QString newStr)
{
    if(m_index == 0 && (m_createdBy != CALLED_BY_VIEWCAM_ADD_LIST))
    {
        if(newStr != m_connectStr)
        {
            m_textLabel->changeText(newStr);
            m_textLabel->update();
            m_connectStr = newStr;
        }
    }
}

CAMERA_STATE_TYPE_e CameraListButton::getConnectionState()
{
    return m_currentConnectionState;
}

void CameraListButton::drawImage(QPainter * painter)
{
    if(m_index != 0 || (m_createdBy == CALLED_BY_VIEWCAM_ADD_LIST))
    {
        painter->drawPixmap((m_mainRect.topLeft().x() + SCALE_WIDTH(15)),
                            m_mainRect.topLeft().y() + SCALE_HEIGHT(3), m_iconImage);
    }
    else
    {
        painter->drawPixmap((m_mainRect.topLeft().x() + SCALE_WIDTH(10)),
                            m_mainRect.topLeft().y() + SCALE_HEIGHT(3), m_iconImage);
    }
}

void CameraListButton::resetGeometry(int xOffset, int yOffset)
{
    this->setGeometry(QRect(xOffset,
                            (m_height * (m_index + yOffset)) ,
                            m_width,
                            m_height));
}
void CameraListButton::resetGeometryCustIndex(int xOffset, int yOffset)
{
    this->setGeometry(QRect(xOffset+10,
                            (m_height * yOffset) ,
                            (m_width),
                            m_height));
}

void CameraListButton::paintEvent(QPaintEvent * event)
{
    MenuButton::paintEvent(event);
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    drawRectangles(&painter);
    drawImage(&painter);
    QWidget::paintEvent(event);
}

void CameraListButton::mouseReleaseEvent(QMouseEvent * event)
{
    if((m_mouseClicked)
            && (event->button() == m_leftMouseButton)
            && (!m_isDeletionStart))
    {
        takeEnterKeyAction();
    }
    m_mouseClicked = false;
}

void CameraListButton::mousePressEvent(QMouseEvent * event)
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

void CameraListButton::slotClickEffectTimerout()
{
    setShowClickedImage(false);
    emit sigButtonClicked(m_index, m_currentConnectionState,m_deviceIndex);
}

int CameraListButton::getDeviceIndex()
{
   return m_deviceIndex;
}
