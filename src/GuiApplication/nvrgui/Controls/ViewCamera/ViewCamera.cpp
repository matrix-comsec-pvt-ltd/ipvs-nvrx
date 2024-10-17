#include "ViewCamera.h"
#include <QDesktopWidget>
#include <QKeyEvent>

#define VIEW_CAMERA_LIST_WIDTH       (SCALE_WIDTH(320) + SCALE_WIDTH(58))
#define VIEW_CAMERA_LIST_HEIGHT      (SCALE_HEIGHT(760) + SCALE_HEIGHT(80))

ViewCamera::ViewCamera(QWidget *parent, quint16 windowIndex)
    :  BackGround((ApplController::getXPosOfScreen() + ((ApplController::getWidthOfScreen() - VIEW_CAMERA_LIST_WIDTH) / 2)),
          (ApplController::getYPosOfScreen() + ((ApplController::getHeightOfScreen() - VIEW_CAMERA_LIST_HEIGHT) / 2)),
          VIEW_CAMERA_LIST_WIDTH,
          VIEW_CAMERA_LIST_HEIGHT,
          BACKGROUND_TYPE_4,
          MAX_TOOLBAR_BUTTON,
          parent, true,
          "View Camera")
{
    m_elementList[0] = m_mainCloseButton;
    connect(m_mainCloseButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    viewCameraAddList = new CameraList(SCALE_WIDTH(20),
                                       SCALE_HEIGHT(55),
                                       this,
                                       0,
                                       CALLED_BY_VIEWCAM_ADD_LIST,
                                       windowIndex);

    m_elementList[1] = viewCameraAddList;
    connect(viewCameraAddList,
            SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
            this,
            SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
    connect(viewCameraAddList,
            SIGNAL(sigSwapWindows(quint16,quint16)),
            this,
            SLOT(slotSwapWindows(quint16,quint16)));
    connect(viewCameraAddList,
            SIGNAL(sigStartStreamInWindow(DISPLAY_TYPE_e,QString,quint8,quint16)),
            this,
            SLOT(slotStartStreamInWindow(DISPLAY_TYPE_e,QString,quint8,quint16)));
    connect(viewCameraAddList,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    this->show();

    m_currentElement = 1;
    m_elementList[m_currentElement]->forceFocusToPage(true);
}

ViewCamera::~ViewCamera()
{
    disconnect(viewCameraAddList,
               SIGNAL(sigClosePage(TOOLBAR_BUTTON_TYPE_e)),
               this,
               SLOT(slotClosePage(TOOLBAR_BUTTON_TYPE_e)));
    disconnect(viewCameraAddList,
               SIGNAL(sigSwapWindows(quint16,quint16)),
               this,
               SLOT(slotSwapWindows(quint16,quint16)));
    disconnect(viewCameraAddList,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect(viewCameraAddList,
               SIGNAL(sigStartStreamInWindow(DISPLAY_TYPE_e,QString,quint8,quint16)),
               this,
               SLOT(slotStartStreamInWindow(DISPLAY_TYPE_e,QString,quint8,quint16)));
    delete viewCameraAddList;

    disconnect(m_mainCloseButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
}

void ViewCamera::updateDeviceState(QString deviceName, DEVICE_STATE_TYPE_e devState)
{
    viewCameraAddList->updateDeviceCurrentState(deviceName, devState);
}

void ViewCamera::takeUpKeyAction()
{
    do
    {
        m_currentElement = (m_currentElement - 1 + MAX_ELEMENTS) % MAX_ELEMENTS;
    }while((m_elementList[m_currentElement] == NULL)
           || (!m_elementList[m_currentElement]->getIsEnabled()));

    if(m_currentElement == 0)
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }
    else
    {
        m_elementList[m_currentElement]->forceFocusToPage(true);
    }
}

void ViewCamera::takeDownKeyAction()
{
    do
    {
        m_currentElement = (m_currentElement + 1) % MAX_ELEMENTS;
    }while((m_elementList[m_currentElement] == NULL)
           || (!m_elementList[m_currentElement]->getIsEnabled()));

    if(m_currentElement == 0)
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }
    else
    {
        m_elementList[m_currentElement]->forceFocusToPage(true);
    }
}

void ViewCamera::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void ViewCamera::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    m_currentElement = 0;
    m_elementList[m_currentElement]->forceActiveFocus();
}

void ViewCamera::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeDownKeyAction();
}

void ViewCamera::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeUpKeyAction();
}

void ViewCamera::showEvent(QShowEvent *event)
{
    QWidget::showEvent (event);
    m_elementList[m_currentElement]->forceActiveFocus();
}

void ViewCamera::slotClosePage(TOOLBAR_BUTTON_TYPE_e)
{
    emit sigClosePage(MAX_TOOLBAR_BUTTON);
}

void ViewCamera::slotSwapWindows(quint16 firstWindow, quint16 secondWindow)
{
    emit sigSwapWindows(firstWindow, secondWindow);
}

void ViewCamera::slotUpdateCurrentElement(int indexInPage)
{
    m_currentElement = indexInPage;
}

void ViewCamera::slotStartStreamInWindow(DISPLAY_TYPE_e displayType,
                                         QString deviceName,
                                         quint8 channelId,
                                         quint16 windowId)
{
    emit sigStartStreamInWindow(displayType, deviceName, channelId, windowId);
}
