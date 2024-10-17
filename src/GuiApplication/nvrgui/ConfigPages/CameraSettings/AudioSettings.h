#ifndef AUDIOSETTINGS_H
#define AUDIOSETTINGS_H

#include "Controls/ConfigPageControl.h"
#include "Controls/ElementHeading.h"
#include "Controls/DropDown.h"
#include "Controls/TextLabel.h"

class AudioSettings : public ConfigPageControl
{
public:
    explicit AudioSettings(QString deviceName, QWidget *parent = 0,DEV_TABLE_INFO_t *devTabInfo = NULL);
    ~AudioSettings();

    void createPayload(REQ_MSG_ID_e msgType);
    void getConfig();
    void saveConfig();
    void defaultConfig();
    void processDeviceResponse(DevCommParam *param, QString deviceName);

private:
    ElementHeading* audioOutSettingTile;
    DropDown*       audioOutPlay;
};

#endif // AUDIOSETTINGS_H
