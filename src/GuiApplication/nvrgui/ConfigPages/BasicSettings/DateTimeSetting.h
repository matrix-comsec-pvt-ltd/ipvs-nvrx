#ifndef DATETIMESETTING_H
#define DATETIMESETTING_H

#include "Controls/ConfigPageControl.h"
#include "Controls/TextBox.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/ControlButton.h"
#include "Controls/CalendarTile.h"
#include "Controls/Clockspinbox.h"

#include "Controls/DropDown.h"
#include "DataStructure.h"
#include "NavigationControl.h"
#include "ConfigField.h"

typedef enum
{
	SYNC_CAM_TIME_OPTION_LOCAL_TIME,
	SYNC_CAM_TIME_OPTION_UTC_TIME,

	MAX_SYNC_CAM_TIME_OPTION
}SYNC_CAM_TIME_OPTION_e;

class DateTimeSetting : public ConfigPageControl
{
    Q_OBJECT

private:
    DropDown*   m_timeZoneDropDownBox;
    CalendarTile *dateCalTile;
    ClockSpinbox *timeClockSpinBox;
    ControlButton *controlBtn;

    OptionSelectButton *autoSyncOption;
    OptionSelectButton *m_autoUpdateRegionalOption;
    OptionSelectButton *m_UpdateTimeZoneOption;
	OptionSelectButton*		m_SyncCameraTimeOptButton[MAX_SYNC_CAM_TIME_OPTION];
	SYNC_CAM_TIME_OPTION_e	m_SyncCameraTimeOpt;
	TextboxParam			*otherSerTextboxParam;
    TextBox *otherSerTextbox;

    DropDown*   updateIntervalDropDownBox;

    quint8 serverIndex;
    quint16 m_regvideoStandard;
    quint16 m_regdateFormat;
    quint16 m_regintegrateCosec;
    bool    m_dateStatus;

public:
    explicit DateTimeSetting(QString devName,
                   QWidget *parent = 0,
                   DEV_TABLE_INFO_t* devTable = NULL);
    ~DateTimeSetting();

    void createDefaultComponent();
    void processDeviceResponse(DevCommParam *param, QString deviceName);

    void getConfig();
    void defaultConfig();
    void saveConfig();

    void getDevDateTime();
    void updateDateTime(QString dateTimeStr);

    void getRegionalConfig();
    void getRegionalCmdRequest();
    void setConfig(bool generalSettingUpdate = true);
    void handleInfoPageMessage(int index);
	void updateSyncCameraTimeOptionButton(void);
public slots:
    void slotTimeDateSetBtnClicked(int index);
    void slotAutoSyncButtonClicked(OPTION_STATE_TYPE_e state,int);
    void slotNtpServerChanged(QString string, quint32 indexInPage);
	void slotSyncCameraTimeOptionButtonClicked(OPTION_STATE_TYPE_e state,int index);
    void slotDateChanged(void);       
};

#endif // DATETIMESETTING_H
