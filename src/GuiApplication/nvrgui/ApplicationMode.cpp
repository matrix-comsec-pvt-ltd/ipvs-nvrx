#include "ApplicationMode.h"

ApplicationMode* ApplicationMode::applicationMode = NULL;
ApplicationMode::ApplicationMode(QObject *parent) :
    QObject(parent)
{
    applicationModeType = MODE_NONE;
}

ApplicationMode* ApplicationMode::getApplicationModeInstance(QObject *parent)
{
    if(applicationMode == NULL)
    {
        applicationMode = new ApplicationMode(parent);
    }
    return applicationMode;
}

void ApplicationMode::setApplicationMode(APPLICATION_MODE_e mode)
{
    applicationMode->setMode(mode);
}

APPLICATION_MODE_e ApplicationMode::getApplicationMode()
{
    return applicationMode->getMode();
}

void ApplicationMode::setMode(APPLICATION_MODE_e mode)
{
    if(applicationModeType != mode)
    {
        applicationModeType = mode;
        emit sigApplicationModeChanged();
    }
}

APPLICATION_MODE_e ApplicationMode::getMode()
{
    return applicationModeType;
}
