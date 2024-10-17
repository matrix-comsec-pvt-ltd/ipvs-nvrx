#ifndef DATEANDTIME_H
#define DATEANDTIME_H


#include "Controls/ElementHeading.h"
#include "EnumFile.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/DropDown.h"
#include "Controls/ConfigPageControl.h"

#include "Controls/DropDown.h"
#include "NavigationControl.h"
#include "ConfigField.h"
#include "Controls/CalendarTile.h"
#include "Controls/Clockspinbox.h"
#include "Controls/ControlButton.h"
#include "Controls/TextBox.h"
#include "PayloadLib.h"
#include "WizardCommon.h"
#include "ConfigPages/BasicSettings/DateTimeSetting.h"

class DateAndTime : public WizardCommon
{
    Q_OBJECT
private:
    TextLabel*              m_sub_pageHeading;
    DropDown*               m_timeZoneDropDownBox;
    DropDown*               m_languageDropDownBox;
    CalendarTile*           dateCalTile;
    ClockSpinbox*           timeClockSpinBox;
    ControlButton*          controlBtn;

    OptionSelectButton*     autoSyncOption;
    OptionSelectButton*     m_autoUpdateRegionalOption;
    OptionSelectButton*     m_UpdateTimeZoneOption;
    TextboxParam*           otherSerTextboxParam;
    TextBox*                otherSerTextbox;
	OptionSelectButton*		m_SyncCameraTimeOptButton[MAX_SYNC_CAM_TIME_OPTION];
	SYNC_CAM_TIME_OPTION_e	m_SyncCameraTimeOpt;

    DropDown*               updateIntervalDropDownBox;
    ApplController*         applController;
    PayloadLib*             payloadLib;
    InfoPage*               m_infoPage;

    quint8                  serverIndex;
    QString                 currDevName;
    bool                    isErrRespInfoPage;  /* Don't apply language settings on error response info page */
    QString                 selectedLangStr;
    bool                    m_dateStatus;

public:

    explicit DateAndTime(QString devName, QString subHeadStr, QWidget *parent = 0, WIZARD_PAGE_INDEXES_e pageId = MAX_WIZ_PG);

    virtual ~DateAndTime();

    void defaultConfig();
    void processDeviceResponse(DevCommParam *param, QString deviceName);
    void getConfig();
    void getLanguage(void);
    void getDevDateTime();
    void updateDateTime(QString dateTimeStr);
    void saveConfig();
    void handleInfoPageMessage (int index);
	void updateSyncCameraTimeOptionButton(void);
    void getUserPreferredLanguage(void);
    void setUserPreferredLanguage(void);

public slots:

    void slotInfoPageBtnclick(int index);
    void slotTimeDateSetBtnClicked(int index);    
    void slotDropDownBoxValueChanged(QString string, quint32);
    void slotAutoSyncButtonClicked(OPTION_STATE_TYPE_e state,int);
	void slotSyncCameraTimeOptionButtonClicked(OPTION_STATE_TYPE_e state,int index);
    void slotDateChanged(void);
};

#endif // DATEANDTIME_H
