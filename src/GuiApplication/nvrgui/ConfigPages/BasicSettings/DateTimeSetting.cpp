#include "DateTimeSetting.h"
#include "ValidationMessage.h"

#define CNFG_TO_INDEX               1

#define FIRST_ELE_XOFFSET           SCALE_WIDTH(13)
#define FIRST_ELE_YOFFSET           SCALE_HEIGHT(170)

// List of control
typedef enum
{
    DTTIME_STG_CHECKBOX_AUTO_REGIONAL,
    DTTIME_STG_TIME_ZONE,
    DTTIME_STG_DATE,
    DTTIME_STG_TIME,
    DTTIME_STG_SET_BTN,
    DTTIME_STG_CHECKBOX_AUTO,
    DTTIME_STG_NTP_SERVER,
    DTTIME_STG_UPDATE_INTERVAL,
    DTTIME_STG_UPDATETIME,
	DTTIME_STG_SYNC_CAM_TIME_OPT_LOCAL_TIME,
	DTTIME_STG_SYNC_CAM_TIME_OPT_UTC_TIME,

    MAX_DTTIME_STG_ELEMETS
}DTTIME_STG_ELELIST_e;

typedef enum
{
    NTP_OTHER,
    NTP_WISCONSIN,
    NTP_WINDOWS,
    NTP_NIST
}NTP_TYPE_e;

static const QString labelStr[MAX_DTTIME_STG_ELEMETS] =
{
    "Auto Update Regional Settings",
    "Time Zone",
    "Date",
    "Time",
    "Set",
    "Auto Synchronize with NTP Server",
    "Preferred NTP Server",
    "Update",
    "Synchronize NVR Time Zone to ONVIF Cameras",
	"Local Time",
	"UTC Time"
};

static const QStringList timezoneArray = QStringList()
        << "(GMT-12:00) International Date Line West"           //0
        << "(GMT-11:00) Midway Island, Samoa"
        << "(GMT-10:00) Hawaii"
        << "(GMT-09:00) Alaska"
        << "(GMT-08:00) Pacific Time (US and Canada); Tijuana"
        << "(GMT-07:00) Arizona"
        << "(GMT-07:00) Chihuahua, La Paz, Mazatlan"
        << "(GMT-07:00) Mountain Time (US and Canada)"
        << "(GMT-06:00) Central America"
        << "(GMT-06:00) Central Time (US and Canada)"
        << "(GMT-06:00) Guadalajara, Mexico City, Monterrey"    //10
        << "(GMT-06:00) Saskatchewan"
        << "(GMT-05:00) Bogota, Lima, Quito"
        << "(GMT-05:00) Eastern Time (US and Canada)"
        << "(GMT-05:00) Indiana (East)"
        << "(GMT-04:00) Atlantic Time (Canada)"
        << "(GMT-04:00) Caracas, La Paz"
        << "(GMT-04:00) Santiago"
        << "(GMT-03:30) Newfoundland"
        << "(GMT-03:00) Brasilia"
        << "(GMT-03:00) Buenos Aires, Georgetown"               //20
        << "(GMT-03:00) Greenland"
        << "(GMT-02:00) Mid-Atlantic"
        << "(GMT-01:00) Azores"
        << "(GMT-01:00) Cape Verde Is"
        << "(GMT) CASABLANCA, MONROVIA"
        << "(GMT) Dublin, Edinburgh, Lisbon, London"
        << "(GMT+01:00) Amsterdam, Berlin, Bern, Rome, Stockholm, Vienna"
        << "(GMT+01:00) Belgrade, Bratislava, Budapest, Ljubljana, Prague"
        << "(GMT+01:00) Brussels, Copenhagen, Madrid, Paris"
        << "(GMT+01:00) Sarajevo, Skopje, Warsaw, Zagreb"       //30
        << "(GMT+01:00) West Central Africa"
        << "(GMT+02:00) Athens, Beirut, Istanbul, Minsk"
        << "(GMT+02:00) Bucharest"
        << "(GMT+02:00) Cairo"
        << "(GMT+02:00) Harare, Pretoria"
        << "(GMT+02:00) Helsinki, Kyiv, Riga, Sofia, Tallinn, Vilnius"
        << "(GMT+02:00) Jerusalem"
        << "(GMT+03:00) Baghdad"
        << "(GMT+03:00) Kuwait, Riyadh"
        << "(GMT+03:00) Moscow, St. Petersburg, Volgograd"      //40
        << "(GMT+03:00) Nairobi"
        << "(GMT+03:30) Tehran"
        << "(GMT+04:00) Abu Dhabi, Muscat"
        << "(GMT+04:00) Baku, Tbilisi, Yerevan"
        << "(GMT+04:30) Kabul"
        << "(GMT+05:00) Ekaterinburg"
        << "(GMT+05:00) Islamabad, Karachi, Tashkent"
        << "(GMT+05:30) Chennai, Kolkata, Mumbai, New Delhi"
        << "(GMT+05:45) Kathmandu"
        << "(GMT+06:00) Almay, Novosibirsk"                    //50
        << "(GMT+06:00) Astana, Dhaka"
        << "(GMT+06:00) Sri Jayewardenepura"
        << "(GMT+06:30) Rangoon"
        << "(GMT+07:00) Bangkok, Hanoi, Jakarta"
        << "(GMT+07:00) Krasnoyarsk"
        << "(GMT+08:00) Beijing, Chongqing, Hong Kong, Urumqi"
        << "(GMT+08:00) Irkutsk, Ulaan Bataar"
        << "(GMT+08:00) Kuala Lumpur, Singapore"
        << "(GMT+08:00) Perth"
        << "(GMT+08:00) Taipei"                                  //60
        << "(GMT+09:00) Osaka, Sapporo, Tokyo"
        << "(GMT+09:00) Seoul"
        << "(GMT+09:00) Yakutsk"
        << "(GMT+09:30) Adelaide"
        << "(GMT+09:30) Darwin"
        << "(GMT+10:00) Brisbane"
        << "(GMT+10:00) Canberra, Sydney, Melbourne"
        << "(GMT+10:00) Guam, Port Moresby"
        << "(GMT+10:00) Hobart"
        << "(GMT+10:00) Vladivostok"                        //70
        << "(GMT+11:00) Magadan, Solomon Is, New Caledonia"
        << "(GMT+12:00) Auckland, Wellington"
        << "(GMT+12:00) Fiji, Kamchatka, Marshall Is"
        << "(GMT+13:00) Nuku'alofa";                            // 74 :frm 0

static const QStringList ntpSerOption = QStringList () << "Other"
                                                       << "Wisconsin"
                                                       << "Window"
                                                       << "NIST";

static QStringList updateIntStrList;
//*****************************************************************************
// createDefaultElement
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
DateTimeSetting::DateTimeSetting(QString devName, QWidget *parent,DEV_TABLE_INFO_t *devTable)
    :ConfigPageControl(devName, parent, MAX_DTTIME_STG_ELEMETS,devTable), serverIndex(0), m_regvideoStandard(0)
    ,m_regdateFormat(0), m_regintegrateCosec(0), m_dateStatus(false)
{
	m_SyncCameraTimeOpt = SYNC_CAM_TIME_OPTION_LOCAL_TIME;
	updateIntStrList = QStringList () << QString("6 ") + Multilang("Hours Interval")
                                          << QString("12 ") + Multilang("Hours Interval")
                                          << QString("24 ") + Multilang("Hours Interval");
    createDefaultComponent();
    DateTimeSetting::getConfig();
}
//*****************************************************************************
// createDefaultElement
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
DateTimeSetting::~DateTimeSetting()
{

    if(IS_VALID_OBJ(m_UpdateTimeZoneOption))
    {
        disconnect (m_UpdateTimeZoneOption,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_UpdateTimeZoneOption);
    }

	if(IS_VALID_OBJ(m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_LOCAL_TIME]))
	{
		disconnect (m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_LOCAL_TIME],
					SIGNAL(sigUpdateCurrentElement(int)),
					this,
					SLOT(slotUpdateCurrentElement(int)));
		disconnect (m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_LOCAL_TIME],
					SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
					this,
					SLOT(slotSyncCameraTimeOptionButtonClicked(OPTION_STATE_TYPE_e,int)));
		DELETE_OBJ(m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_LOCAL_TIME]);
	}
	if(IS_VALID_OBJ(m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_UTC_TIME]))
	{
		disconnect (m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_UTC_TIME],
					SIGNAL(sigUpdateCurrentElement(int)),
					this,
					SLOT(slotUpdateCurrentElement(int)));
		disconnect (m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_UTC_TIME],
					SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
					this,
					SLOT(slotSyncCameraTimeOptionButtonClicked(OPTION_STATE_TYPE_e,int)));
		DELETE_OBJ(m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_UTC_TIME]);
	}

    if(IS_VALID_OBJ(m_timeZoneDropDownBox))
    {
        disconnect (m_timeZoneDropDownBox,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_timeZoneDropDownBox);
    }

    disconnect (dateCalTile,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect(dateCalTile,
               SIGNAL(sigDateChanged()),
               this,
               SLOT(slotDateChanged()));
    delete dateCalTile;

    disconnect (timeClockSpinBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete timeClockSpinBox;

    disconnect (controlBtn,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect(controlBtn,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotTimeDateSetBtnClicked(int)));
    delete controlBtn;

    disconnect (autoSyncOption,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect(autoSyncOption,
               SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
               this,
               SLOT(slotAutoSyncButtonClicked(OPTION_STATE_TYPE_e,int)));
    delete autoSyncOption;

    disconnect (otherSerTextbox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete otherSerTextbox;
    delete otherSerTextboxParam;

    disconnect (updateIntervalDropDownBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete updateIntervalDropDownBox;


    if(IS_VALID_OBJ(m_autoUpdateRegionalOption))
    {
        disconnect (m_autoUpdateRegionalOption,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_autoUpdateRegionalOption);
    }
}
//*****************************************************************************
// createDefaultElement
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void DateTimeSetting::createDefaultComponent ()
{
    QMap<quint8, QString>  timezoneArrayList;

    for(quint8 index = 0; index < timezoneArray.length (); index++)
    {
        timezoneArrayList.insert (index,timezoneArray.at (index));
    }

    m_autoUpdateRegionalOption = new OptionSelectButton(FIRST_ELE_XOFFSET,
                                                        SCALE_HEIGHT(130),
                                                        BGTILE_LARGE_SIZE_WIDTH,
                                                        BGTILE_HEIGHT,
                                                        CHECK_BUTTON_INDEX,
                                                        labelStr[DTTIME_STG_CHECKBOX_AUTO_REGIONAL],
                                                        this,
														UP_LAYER,
														-1,
														MX_OPTION_TEXT_TYPE_LABEL,
                                                        NORMAL_FONT_SIZE,
                                                        DTTIME_STG_CHECKBOX_AUTO_REGIONAL, -1,
                                                        NORMAL_FONT_COLOR, true);

    m_elementList[DTTIME_STG_CHECKBOX_AUTO_REGIONAL] = m_autoUpdateRegionalOption;

    if(IS_VALID_OBJ(m_autoUpdateRegionalOption))
    {
        connect (m_autoUpdateRegionalOption,
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
    }

	m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_LOCAL_TIME] = new OptionSelectButton(FIRST_ELE_XOFFSET,
																m_autoUpdateRegionalOption->y () + m_autoUpdateRegionalOption->height (),
																BGTILE_LARGE_SIZE_WIDTH,
																BGTILE_HEIGHT,
																RADIO_BUTTON_INDEX,
																this, UP_LAYER,
																"Synchronize Camera Time With",
																labelStr[DTTIME_STG_SYNC_CAM_TIME_OPT_LOCAL_TIME],
																-1,
																DTTIME_STG_SYNC_CAM_TIME_OPT_LOCAL_TIME,
																true,
																NORMAL_FONT_SIZE,
																NORMAL_FONT_COLOR);

	m_elementList[DTTIME_STG_SYNC_CAM_TIME_OPT_LOCAL_TIME] = m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_LOCAL_TIME];
	connect (m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_LOCAL_TIME],
			 SIGNAL(sigUpdateCurrentElement(int)),
			 this,
			 SLOT(slotUpdateCurrentElement(int)));

	connect (m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_LOCAL_TIME],
			 SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
			 this,
			 SLOT(slotSyncCameraTimeOptionButtonClicked(OPTION_STATE_TYPE_e,int)));

	m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_UTC_TIME] = new OptionSelectButton(m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_LOCAL_TIME]->x() + SCALE_WIDTH(650),
																m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_LOCAL_TIME]->y (),
																BGTILE_LARGE_SIZE_WIDTH,
																BGTILE_HEIGHT,
																RADIO_BUTTON_INDEX,
																labelStr[DTTIME_STG_SYNC_CAM_TIME_OPT_UTC_TIME],
																this,
																NO_LAYER,
																SCALE_WIDTH(0),
																MX_OPTION_TEXT_TYPE_SUFFIX,
																NORMAL_FONT_SIZE,
																DTTIME_STG_SYNC_CAM_TIME_OPT_UTC_TIME, -1,
																NORMAL_FONT_COLOR, true);

	m_elementList[DTTIME_STG_SYNC_CAM_TIME_OPT_UTC_TIME] = m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_UTC_TIME];
	connect (m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_UTC_TIME],
			 SIGNAL(sigUpdateCurrentElement(int)),
			 this,
			 SLOT(slotUpdateCurrentElement(int)));

	connect (m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_UTC_TIME],
			 SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
			 this,
			 SLOT(slotSyncCameraTimeOptionButtonClicked(OPTION_STATE_TYPE_e,int)));

    m_UpdateTimeZoneOption = new OptionSelectButton(FIRST_ELE_XOFFSET,
														m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_LOCAL_TIME]->y () + m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_LOCAL_TIME]->height (),
                                                        BGTILE_LARGE_SIZE_WIDTH,
                                                        BGTILE_HEIGHT,
                                                        CHECK_BUTTON_INDEX,
                                                        labelStr[DTTIME_STG_UPDATETIME],
                                                        this,
														UP_LAYER,
														-1,
														MX_OPTION_TEXT_TYPE_LABEL,
                                                        NORMAL_FONT_SIZE,
                                                        DTTIME_STG_UPDATETIME, -1,
                                                        NORMAL_FONT_COLOR, true);

    m_elementList[DTTIME_STG_UPDATETIME] = m_UpdateTimeZoneOption;

    if(IS_VALID_OBJ(m_UpdateTimeZoneOption))
    {
        connect (m_UpdateTimeZoneOption,
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
    }

    m_timeZoneDropDownBox = new DropDown(m_UpdateTimeZoneOption->x (),
                                         m_UpdateTimeZoneOption->y () + m_UpdateTimeZoneOption->height (),
                                         m_UpdateTimeZoneOption->width(),
                                         BGTILE_HEIGHT,
                                         DTTIME_STG_TIME_ZONE,
                                         DROPDOWNBOX_SIZE_755,
                                         labelStr[DTTIME_STG_TIME_ZONE],
                                         timezoneArrayList,
										 this, "", false, SCALE_WIDTH(60));

    m_elementList[DTTIME_STG_TIME_ZONE] = m_timeZoneDropDownBox;

    if(IS_VALID_OBJ(m_timeZoneDropDownBox))
    {
        connect (m_timeZoneDropDownBox,
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
    }

    dateCalTile = new CalendarTile(m_timeZoneDropDownBox->x (),
                                   m_timeZoneDropDownBox->y () + m_timeZoneDropDownBox->height () + SCALE_HEIGHT(6),
                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                   BGTILE_HEIGHT, "",
                                   labelStr[DTTIME_STG_DATE],
                                   this, DTTIME_STG_DATE,
                                   true, 0,
                                   TOP_TABLE_LAYER);

    m_elementList[DTTIME_STG_DATE] = dateCalTile;

    connect (dateCalTile,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(dateCalTile,
            SIGNAL(sigDateChanged()),
            this,
            SLOT(slotDateChanged()));


    timeClockSpinBox = new ClockSpinbox(m_timeZoneDropDownBox->x (),
                                        dateCalTile->y () + dateCalTile->height (),
                                        BGTILE_MEDIUM_SIZE_WIDTH,
                                        BGTILE_HEIGHT,
                                        DTTIME_STG_TIME,
                                        CLK_SPINBOX_With_SEC,
                                        labelStr[DTTIME_STG_TIME], 10,
                                        this, "", true,
                                        0,
                                        MIDDLE_TABLE_LAYER);

    m_elementList[DTTIME_STG_TIME] = timeClockSpinBox;

    connect (timeClockSpinBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    controlBtn = new ControlButton(SET_BUTTON_INDEX,
                                   m_timeZoneDropDownBox->x (),
                                   timeClockSpinBox->y () + timeClockSpinBox->height (),
                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                   BGTILE_HEIGHT,
                                   this, BOTTOM_TABLE_LAYER, -1,
                                   labelStr[DTTIME_STG_SET_BTN],
                                   true,
                                   DTTIME_STG_SET_BTN);

    m_elementList[DTTIME_STG_SET_BTN] = controlBtn;

    connect(controlBtn,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotTimeDateSetBtnClicked(int)));
    connect (controlBtn,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    autoSyncOption = new OptionSelectButton((dateCalTile->x() + dateCalTile->width() + SCALE_WIDTH(6)),
                                            (m_timeZoneDropDownBox->y() + m_timeZoneDropDownBox->height() + SCALE_HEIGHT(6)),
                                            BGTILE_MEDIUM_SIZE_WIDTH,
                                            BGTILE_HEIGHT,
                                            CHECK_BUTTON_INDEX,
                                            labelStr[DTTIME_STG_CHECKBOX_AUTO],
                                            this,
                                            TOP_LAYER,
                                            SCALE_WIDTH(20),
                                            MX_OPTION_TEXT_TYPE_SUFFIX,
                                            NORMAL_FONT_SIZE,
                                            DTTIME_STG_CHECKBOX_AUTO);
    m_elementList[DTTIME_STG_CHECKBOX_AUTO] = autoSyncOption;

    connect(autoSyncOption,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotAutoSyncButtonClicked(OPTION_STATE_TYPE_e,int)));
    connect (autoSyncOption,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    otherSerTextboxParam = new TextboxParam();
    otherSerTextboxParam->labelStr = labelStr[DTTIME_STG_NTP_SERVER];
    otherSerTextboxParam->maxChar = 40;
    otherSerTextboxParam->validation = QRegExp(asciiset1ValidationString);
    otherSerTextboxParam->isTotalBlankStrAllow = true;

    otherSerTextbox = new TextBox(autoSyncOption->x (),
                                  autoSyncOption->y () + autoSyncOption->height (),
                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                  BGTILE_HEIGHT,
                                  DTTIME_STG_NTP_SERVER,
                                  TEXTBOX_ULTRALARGE,
                                  this, otherSerTextboxParam,
                                  MIDDLE_TABLE_LAYER, false,
                                  false, false, SCALE_WIDTH(45));

    m_elementList[DTTIME_STG_NTP_SERVER] = otherSerTextbox;

    connect (otherSerTextbox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    QMap<quint8, QString>  updateIntStrMapList;

    for(quint8 index = 0; index < updateIntStrList.length (); index++)
    {
        updateIntStrMapList.insert (index,updateIntStrList.at (index));
    }

    updateIntervalDropDownBox = new DropDown(autoSyncOption->x (),
                                             otherSerTextbox->y ()+otherSerTextbox->height (),
                                             BGTILE_MEDIUM_SIZE_WIDTH,
                                             BGTILE_HEIGHT,
                                             DTTIME_STG_UPDATE_INTERVAL,
                                             DROPDOWNBOX_SIZE_200,
                                             labelStr[DTTIME_STG_UPDATE_INTERVAL],
                                             updateIntStrMapList,
                                             this, "", true, 0,
                                             BOTTOM_TABLE_LAYER, false, 8,
                                             false, false, 5, SCALE_WIDTH(45));

    m_elementList[DTTIME_STG_UPDATE_INTERVAL] = updateIntervalDropDownBox;

    connect (updateIntervalDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

}
//*****************************************************************************
// createDefaultElement
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void DateTimeSetting::getConfig()
{
    //create the payload for Get Cnfg
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                             DATETIME_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_TO_INDEX,
                                                             CNFG_FRM_FIELD,
                                                             MAX_DATE_TIME_FIELD_NO,
                                                             0);

    DevCommParam* param = new DevCommParam();

    param->msgType = MSG_GET_CFG;
    param->payload = payloadString;

    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void DateTimeSetting::getDevDateTime()
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = GET_DATE_TIME;

    applController->processActivity(currDevName, DEVICE_COMM, param);
}

//*****************************************************************************
// createDefaultElement
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void DateTimeSetting::defaultConfig()
{
    //create the payload for Get Cnfg
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_DEF_CFG,
                                                             DATETIME_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_TO_INDEX,
                                                             CNFG_FRM_FIELD,
                                                             MAX_DATE_TIME_FIELD_NO,
                                                             0);

    DevCommParam* param = new DevCommParam();

    param->msgType = MSG_DEF_CFG;
    param->payload = payloadString;

    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

//*****************************************************************************
// createDefaultElement
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************

void DateTimeSetting::saveConfig()
{
    if(m_autoUpdateRegionalOption->getCurrentState () == ON_STATE)      // if Flag is ON then only automatic
    {                                                                   // changes required
        getRegionalCmdRequest();
    }
    else
    {
        setConfig(false);
    }
}

void DateTimeSetting::setConfig(bool generalSettingUpdate)
{
    payloadLib->setCnfgArrayAtIndex (FIELD_TIME_ZONE,
                                     (m_timeZoneDropDownBox->getIndexofCurrElement () + 1));

    payloadLib->setCnfgArrayAtIndex (FIELD_NTP_AUTO_SYNC,
                                     autoSyncOption->getCurrentState ());
    payloadLib->setCnfgArrayAtIndex (FIELD_NTP_SERVER,
                                     serverIndex);
    payloadLib->setCnfgArrayAtIndex (FIELD_OTHER_SERVER,
                                     otherSerTextbox->getInputText ());
    payloadLib->setCnfgArrayAtIndex (FIELD_UPDATE_INTERVAL,
                                     updateIntervalDropDownBox->getIndexofCurrElement ());

    if(IS_VALID_OBJ(m_autoUpdateRegionalOption))
    {
        payloadLib->setCnfgArrayAtIndex (FIELD_AUTO_UPDATE_REGIONAL,
                                         m_autoUpdateRegionalOption->getCurrentState ());
    }
    if(IS_VALID_OBJ(m_UpdateTimeZoneOption))
    {
        payloadLib->setCnfgArrayAtIndex (FIELD_AUTO_UPDATE_TIMEZONE,
                                         m_UpdateTimeZoneOption->getCurrentState ());
    }

	payloadLib->setCnfgArrayAtIndex (FIELD_SYNC_CAM_TIME_OPTION, m_SyncCameraTimeOpt);
    payloadLib->setCnfgArrayAtIndex (MAX_DATE_TIME_FIELD_NO,m_regvideoStandard);
    payloadLib->setCnfgArrayAtIndex (MAX_DATE_TIME_FIELD_NO + 1 ,m_regintegrateCosec);
    payloadLib->setCnfgArrayAtIndex (MAX_DATE_TIME_FIELD_NO + 2,m_regdateFormat);

    //create the payload for Set Cnfg
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_SET_CFG,
                                                             DATETIME_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_TO_INDEX,
                                                             CNFG_FRM_FIELD,
                                                             MAX_DATE_TIME_FIELD_NO,
                                                             MAX_DATE_TIME_FIELD_NO);
    // For auto update Regional flage is ON than only set General setting param
    if(generalSettingUpdate == true)
    {
        payloadString = payloadLib->createDevCnfgPayload(MSG_SET_CFG,
                                                         GENERAL_TABLE_INDEX,
                                                         CNFG_FRM_INDEX,
                                                         CNFG_TO_INDEX,
                                                         FIELD_VIDEO_STD + 1,
                                                         FIELD_DATE_FORMAT + 1,
                                                         (FIELD_TIME_FORMAT - FIELD_VIDEO_STD),
                                                         payloadString,
                                                         MAX_DATE_TIME_FIELD_NO);
    }

    DevCommParam* param = new DevCommParam();

    param->msgType = MSG_SET_CFG;
    param->payload = payloadString;

    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

// Command for getting general page Video standard and date format parameters
void DateTimeSetting::getRegionalConfig()
{
    QString payloadString =
            payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                             GENERAL_TABLE_INDEX,
                                             CNFG_FRM_INDEX,
                                             CNFG_TO_INDEX,
                                             FIELD_VIDEO_STD + 1,
                                             FIELD_DATE_FORMAT + 1,0);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_GET_CFG;
    param->payload = payloadString;

    applController->processActivity(currDevName, DEVICE_COMM, param);
}

// Command for getting TimeZone parameters from Server side
void DateTimeSetting::getRegionalCmdRequest()
{
    //create the payload for Get Regional Cnfg

    payloadLib->setCnfgArrayAtIndex (0,m_timeZoneDropDownBox->getIndexofCurrElement () + 1);

    QString payloadString = payloadLib->createDevCmdPayload(1);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = GET_REG_CFG;
    param->payload = payloadString;

    applController->processActivity(currDevName, DEVICE_COMM, param);
}

//*****************************************************************************
// createDefaultElement
//      Param:
//          IN : Not Applicable
//          OUT: Not Applicable
//      Returns:
//          Not Applicable
//      Description:
//*****************************************************************************
void DateTimeSetting::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    OPTION_STATE_TYPE_e state;

    if(deviceName == currDevName)
    {
        switch(param->deviceStatus)
        {
        case CMD_SUCCESS:
            switch(param->msgType)
            {
            case MSG_GET_CFG:
                payloadLib->parsePayload(param->msgType, param->payload);
                if(payloadLib->getcnfgTableIndex () == DATETIME_TABLE_INDEX)
                {
                    // fill all field value

                    if(IS_VALID_OBJ(m_timeZoneDropDownBox))
                    {
                        m_timeZoneDropDownBox->setIndexofCurrElement (((payloadLib->getCnfgArrayAtIndex (FIELD_TIME_ZONE).toUInt ()) - 1));
                    }


                    state = (OPTION_STATE_TYPE_e)(payloadLib->getCnfgArrayAtIndex (FIELD_NTP_AUTO_SYNC).toUInt ());
                    serverIndex = (payloadLib->getCnfgArrayAtIndex (FIELD_NTP_SERVER).toUInt ());

                    autoSyncOption->changeState (state);

                    otherSerTextbox->setInputText ((payloadLib->getCnfgArrayAtIndex (FIELD_OTHER_SERVER).toString ()));

                    updateIntervalDropDownBox->setIndexofCurrElement
                            (payloadLib->getCnfgArrayAtIndex (FIELD_UPDATE_INTERVAL).toUInt ());

                    if(state == ON_STATE)
                    {
                        updateIntervalDropDownBox->setIsEnabled (true);
                        otherSerTextbox->setIsEnabled (true);
                    }
                    else
                    {
                        otherSerTextbox->setIsEnabled (false);
                        updateIntervalDropDownBox->setIsEnabled (false);
                    }

                    if(IS_VALID_OBJ(m_autoUpdateRegionalOption))
                    {
                        state = (OPTION_STATE_TYPE_e)(payloadLib->getCnfgArrayAtIndex (FIELD_AUTO_UPDATE_REGIONAL).toUInt ());

                        m_autoUpdateRegionalOption->changeState (state);
                    }
                    if(IS_VALID_OBJ(m_UpdateTimeZoneOption))
                    {
                        state = (OPTION_STATE_TYPE_e)(payloadLib->getCnfgArrayAtIndex (FIELD_AUTO_UPDATE_TIMEZONE).toUInt());

                        m_UpdateTimeZoneOption->changeState(state);
                    }

					m_SyncCameraTimeOpt = (SYNC_CAM_TIME_OPTION_e)payloadLib->getCnfgArrayAtIndex (FIELD_SYNC_CAM_TIME_OPTION).toUInt ();
					updateSyncCameraTimeOptionButton();
                    getDevDateTime();
                }

                else if(payloadLib->getcnfgTableIndex () == GENERAL_TABLE_INDEX)
                {
                    m_regintegrateCosec = payloadLib->getCnfgArrayAtIndex (1).toUInt();

                    if(payloadLib->getCnfgArrayAtIndex (0).toUInt() != m_regvideoStandard)
                    {
                      if(devTableInfo->analogCams ==0)
                      {
                        setConfig();
                      }
                      else
                      {
                        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(DATE_TIME_CNG_CFG_VIDEO_STD),true);
                      }
                    }

                    else if(payloadLib->getCnfgArrayAtIndex (2).toUInt () != m_regdateFormat)
                    {
                        setConfig();
                    }
                    else
                    {
                        setConfig(false);
                    }
                }
                break;

            case MSG_SET_CFG:
                //load info page with msg
                processBar->unloadProcessBar();
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
                break;

            case MSG_DEF_CFG:
                processBar->unloadProcessBar();
                getConfig();
                break;

            case MSG_SET_CMD:
                processBar->unloadProcessBar();
                switch(param->cmdType)
                {
                case GET_DATE_TIME:
                    payloadLib->parseDevCmdReply(true, param->payload);
                    updateDateTime(payloadLib->getCnfgArrayAtIndex(0).toString());
                    break;

                case SET_DATE_TIME:
                    //load info page with msg
                    MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(DATE_TIME_SET_SUCCESS));
                    break;

                case GET_REG_CFG:
                    payloadLib->parseDevCmdReply(true, param->payload);

                    m_regvideoStandard = payloadLib->getCnfgArrayAtIndex(0).toUInt();
                    m_regdateFormat = payloadLib->getCnfgArrayAtIndex(1).toUInt();

                    getRegionalConfig();

                    break;

                default:
                    break;
                }
                break;

            default:
                break;
            }
            break;

        default:
            processBar->unloadProcessBar();
            infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
            break;
        }
    }
}

void DateTimeSetting::handleInfoPageMessage(int index)
{
    if((index == INFO_OK_BTN) && (infoPage->getText () == (ValidationMessage::getValidationMessage(DATE_TIME_CNG_CFG_VIDEO_STD).toUtf8().constData())))
    {
        setConfig();
    }
    else
    {
        m_elementList[m_currentElement]->forceActiveFocus ();
    }
}

void DateTimeSetting::updateDateTime(QString dateTimeStr)
{
    //On success response update datestatus
    m_dateStatus = true;
    DDMMYY_PARAM_t param;
    param.date  = dateTimeStr.mid(0, 2).toUInt();
    param.month = dateTimeStr.mid(2, 2).toUInt();
    param.year  = dateTimeStr.mid(4, 4).toUInt();
    dateCalTile->setDDMMYY (&param);

    timeClockSpinBox->assignValue (dateTimeStr.mid(8, 2),
                                   dateTimeStr.mid(10, 2),
                                   dateTimeStr.mid(12, 2));
}

void DateTimeSetting::slotTimeDateSetBtnClicked(int)
{
    // set device Date & time
    // show processing icon
    QString tempStr = "";
    quint32 hour, min, sec;

    if (m_dateStatus == false)
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(INVALID_DATE_RANGE));
        return;
    }

    DDMMYY_PARAM_t *dateParam = dateCalTile->getDDMMYY ();

    if(dateParam->date < 10)
    {
        tempStr.append (QString("%1%2").arg (0).arg (dateParam->date));
    }
    else
    {
        tempStr.append (QString("%1").arg (dateParam->date));
    }

    if(dateParam->month < 10)
    {
        tempStr.append (QString("%1%2").arg (0).arg (dateParam->month));
    }
    else
    {
        tempStr.append (QString("%1").arg (dateParam->month));
    }
    tempStr.append (QString("%1").arg (dateParam->year));

    timeClockSpinBox->currentValue (hour, min, sec);
    if(hour < 10)
    {
        tempStr.append (QString("%1%2").arg (0).arg (hour));
    }
    else
    {
        tempStr.append (QString("%1").arg (hour));
    }

    if(min < 10)
    {
        tempStr.append (QString("%1%2").arg (0).arg (min));
    }
    else
    {
        tempStr.append (QString("%1").arg (min));
    }

    if(sec < 10)
    {
        tempStr.append (QString("%1%2").arg (0).arg (sec));
    }
    else
    {
        tempStr.append (QString("%1").arg (sec));
    }

    payloadLib->setCnfgArrayAtIndex (0, tempStr);
    QString payloadString = payloadLib->createDevCmdPayload(1);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = SET_DATE_TIME;
    param->payload = payloadString;

    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void DateTimeSetting::slotAutoSyncButtonClicked(OPTION_STATE_TYPE_e state,int)
{
    if(state == ON_STATE)
    {
        updateIntervalDropDownBox->setIsEnabled (true);
        otherSerTextbox->setIsEnabled (true);
    }
    else
    {
        otherSerTextbox->setIsEnabled (false);
        updateIntervalDropDownBox->setIsEnabled (false);
    }
}

void DateTimeSetting::slotNtpServerChanged(QString string, quint32)
{
    if(string == ntpSerOption.at (NTP_OTHER))
    {
        otherSerTextbox->setIsEnabled (true);
    }
    else
    {
        otherSerTextbox->setIsEnabled (false);
    }
}

void DateTimeSetting::slotSyncCameraTimeOptionButtonClicked(OPTION_STATE_TYPE_e state,int index)
{
	if(state == ON_STATE)
	{
		switch(index)
		{
		case DTTIME_STG_SYNC_CAM_TIME_OPT_LOCAL_TIME:
			m_SyncCameraTimeOpt = SYNC_CAM_TIME_OPTION_LOCAL_TIME;
			break;

		case DTTIME_STG_SYNC_CAM_TIME_OPT_UTC_TIME:
			m_SyncCameraTimeOpt = SYNC_CAM_TIME_OPTION_UTC_TIME;
			break;

		default:
			break;
		}
		/* Update Button State */
		updateSyncCameraTimeOptionButton();
	}
}

void DateTimeSetting::updateSyncCameraTimeOptionButton(void)
{
	switch(m_SyncCameraTimeOpt)
	{
	case SYNC_CAM_TIME_OPTION_LOCAL_TIME:
		/* Update Button State */
		m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_LOCAL_TIME]->changeState (ON_STATE);
		m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_UTC_TIME]->changeState (OFF_STATE);
		/* On selecting LOCAL_TIME, Make Synchronize NVR Time Zone to ONVIF cameras un-editable */
		m_UpdateTimeZoneOption->setIsEnabled (false);
		break;

	case SYNC_CAM_TIME_OPTION_UTC_TIME:
		/* Update Button State */
		m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_LOCAL_TIME]->changeState (OFF_STATE);
		m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_UTC_TIME]->changeState (ON_STATE);
		/* On selecting UTC_TIME, Make Synchronize NVR Time Zone to ONVIF cameras editable */
		m_UpdateTimeZoneOption->setIsEnabled (true);
		break;

	default:
		break;
	}
}

void DateTimeSetting::slotDateChanged(void)
{
    DDMMYY_PARAM_t *dateParam = dateCalTile->getDDMMYY();

    //Check the year is valid are not
    if ((dateParam->year < 2012) || (dateParam->year > 2037))
    {
        m_dateStatus = false;
        return;
    }

    m_dateStatus = true;
}
