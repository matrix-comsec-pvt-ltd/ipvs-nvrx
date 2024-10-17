#include "BuzzerControl.h"
#include "ApplicationMode.h"
#include "Controls/MessageBanner.h"
#include "ValidationMessage.h"

BuzzerControl::BuzzerControl(QWidget *parent) :
    QWidget(parent)
{
    m_applController = ApplController::getInstance();
    sendStopBuzzerCommand();
}

void BuzzerControl::sendStopBuzzerCommand()
{
    ApplicationMode::setApplicationMode(IDLE_MODE);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = BUZ_CTRL;

    if(!m_applController->processActivity(LOCAL_DEVICE_NAME, DEVICE_COMM, param))
    {
        MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
    }
}

void BuzzerControl::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if(deviceName == LOCAL_DEVICE_NAME)
    {
        switch(param->deviceStatus)
        {
        case CMD_SUCCESS:
            MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(BUZZER_CONTROL_STOP));
            break;
        default:
            MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
            break;
        }
    }
    emit sigClosePage(BUZZER_CONTROL_BUTTON);
}
