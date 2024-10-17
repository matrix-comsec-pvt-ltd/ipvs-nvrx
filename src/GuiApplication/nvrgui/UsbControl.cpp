#include "UsbControl.h"
#include "Controls/MessageBanner.h"
#include "ApplicationMode.h"
#include "ValidationMessage.h"

bool UsbControl::showUsbFlag[MAX_USB_TYPE] = {false};
int UsbControl::currentNoOfUsb = 0;

UsbControl::UsbControl(QWidget *parent) :
    QWidget(parent)
{
    m_bottomY = 0;
    m_startX = 0;
    m_currentClickedUsb = 0;
    ApplicationMode::setApplicationMode(PAGE_WITH_TOOLBAR_MODE);
    m_applController = ApplController::getInstance();
    m_payloadLib = NULL;
    m_payloadLib = new PayloadLib();
    for(int index = 0; index < MAX_USB_TYPE; index++)
    {
        m_usbEjectButton[index] = new MenuButton(index,
                                                 SCALE_WIDTH(260),
                                                 SCALE_HEIGHT(30),
                                                 usbEjectMsg[index],
                                                 this,
                                                 SCALE_WIDTH(20),
                                                 0,
                                                 0,
                                                 index,
                                                 true,
                                                 true,
                                                 false,
                                                 false,
                                                 true);
        m_usbEjectButton[index]->setVisible(showUsbFlag[index]);

        connect(m_usbEjectButton[index],
                SIGNAL(sigButtonClicked(int)),
                this,
                SLOT(slotEjectButtonClicked(int)));
    }
    resetGeometry();
    this->show();
}

UsbControl::~UsbControl()
{
    for(int index = 0; index < MAX_USB_TYPE; index++)
    {
        disconnect(m_usbEjectButton[index],
                   SIGNAL(sigButtonClicked(int)),
                   this,
                   SLOT(slotEjectButtonClicked(int)));

        delete m_usbEjectButton[index];
    }
    if(m_payloadLib != NULL)
    {
        DELETE_OBJ(m_payloadLib);
    }
}


void UsbControl::updateShowUsbFlag(int usbType, bool status)
{
    showUsbFlag[usbType - 1] = status;
    int totalElements = 0;
    for(int index = 0; index < MAX_USB_TYPE; index++)
    {
        if(showUsbFlag[index])
            totalElements++;
    }
    currentNoOfUsb = totalElements;
}

void UsbControl::unPlugUsb(int index)
{
    m_payloadLib->setCnfgArrayAtIndex(0, index);
    QString payload = m_payloadLib->createDevCmdPayload(1);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = USBUNPLUG;
    param->payload = payload;

    m_applController->processActivity(DEVICE_COMM, param);
}

void UsbControl::setCurrentUsbStatus(int usbType, bool status)
{
    m_usbEjectButton[usbType - 1]->setVisible(status);
    resetGeometry();
    if(currentNoOfUsb == 0)
    {
        emit sigClosePage(USB_CONTROL_BUTTON);
    }
}


void UsbControl::resetGeometry()
{
    for(int index = 0; index < MAX_USB_TYPE; index++)
    {
        m_usbEjectButton[index]->resetGeometry(0, 0);
    }

    if((!showUsbFlag[USB_TYPE_MANUAL]) && (showUsbFlag[USB_TYPE_SCHEDULED]))
    {
        m_usbEjectButton[USB_TYPE_SCHEDULED]->resetGeometry(0, -1);
    }

    this->setGeometry((ApplController::getXPosOfScreen() + ApplController::getWidthOfScreen() - SCALE_WIDTH(260)),
                      (ApplController::getYPosOfScreen() + ApplController::getHeightOfScreen() - (SCALE_HEIGHT(30) * currentNoOfUsb) - TOOLBAR_BUTTON_HEIGHT),
                      SCALE_WIDTH(260),
                      (SCALE_HEIGHT(30) * currentNoOfUsb));
}

void UsbControl::slotEjectButtonClicked(int index)
{
    m_currentClickedUsb = index;
    unPlugUsb(index);
}


void UsbControl::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if(deviceName == LOCAL_DEVICE_NAME)
    {
        switch(param->deviceStatus)
        {
        case CMD_SUCCESS:
            updateShowUsbFlag((m_currentClickedUsb + 1), false);
            setCurrentUsbStatus((m_currentClickedUsb + 1), false);
            MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(USB_MANAGE_USB_EJECT_SUCCESS));
            break;
        default:
            MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
            break;
        }
    }
}
