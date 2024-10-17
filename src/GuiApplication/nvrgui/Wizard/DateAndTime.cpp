#include "DateAndTime.h"

#define CNFG_TO_INDEX   1
#define RECT_WIDTH      SCALE_HEIGHT(1000)
#define RECT_HEIGHT     SCALE_HEIGHT(40)

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
        << "(GMT+10:00) Vladivostok"                            //70
        << "(GMT+11:00) Magadan, Solomon Is, New Caledonia"
        << "(GMT+12:00) Auckland, Wellington"
        << "(GMT+12:00) Fiji, Kamchatka, Marshall Is"
        << "(GMT+13:00) Nuku'alofa";                            // 74 :frm 0

static QStringList wizardUpdateIntStrList;

typedef enum
{
    WIZARD_DTTIME_STG_TIME_ZONE,
    WIZARD_DTTIME_STG_OSD_DATE,
    WIZARD_DTTIME_STG_DATE,
    WIZARD_DTTIME_STG_TIME,
    WIZARD_DTTIME_STG_SET_BTN,
    WIZARD_DTTIME_STG_CHECKBOX_AUTO,
    WIZARD_DTTIME_STG_NTP_SERVER,
    WIZARD_DTTIME_STG_UPDATE_INTERVAL,
    WIZARD_DTTIME_STG_LANGUAGE,
    WIZARD_DTTIME_STG_AUTO_UPDATE_TIMEZONE,
	WIZARD_DTTIME_STG_SYNC_CAM_TIME_OPT_LOCAL_TIME,
	WIZARD_DTTIME_STG_SYNC_CAM_TIME_OPT_UTC_TIME,
    MAX_WIZARD_DTTIME_STG_ELEMETS
}WIZARD_DTTIME_STG_ELELIST_e;

static const QString wizardlabelstr[MAX_WIZARD_DTTIME_STG_ELEMETS] =
{
    "Time Zone",
    "OSD Date Format",
    "Date",
    "Time",
    "Set",
    "Auto Synchronize with NTP Server",
    "Preferred NTP Server",
    "Update",
    "Language",
    "Synchronize NVR Time Zone to ONVIF Cameras",
	"Local Time",
	"UTC Time"
};

DateAndTime::DateAndTime(QString devName, QString subHeadStr, QWidget *parent, WIZARD_PAGE_INDEXES_e pageId) : WizardCommon(parent, pageId)
{
    isErrRespInfoPage = false;
    serverIndex = 0;
    m_dateStatus = false;

    INIT_OBJ(m_autoUpdateRegionalOption);
    INIT_OBJ(m_sub_pageHeading);
    INIT_OBJ(m_timeZoneDropDownBox);
    INIT_OBJ(dateCalTile);
    INIT_OBJ(timeClockSpinBox);
    INIT_OBJ(controlBtn);
    INIT_OBJ(autoSyncOption);
    INIT_OBJ(otherSerTextboxParam);
    INIT_OBJ(otherSerTextbox);
    INIT_OBJ(updateIntervalDropDownBox);
    INIT_OBJ(payloadLib);
    INIT_OBJ(m_languageDropDownBox);
    INIT_OBJ(processBar);
    INIT_OBJ(m_infoPage);

    currDevName = devName;
    applController = ApplController::getInstance ();
	m_SyncCameraTimeOpt = SYNC_CAM_TIME_OPTION_LOCAL_TIME;

	WizardCommon:: LoadProcessBar();
    payloadLib = new PayloadLib();

    wizardUpdateIntStrList = QStringList () << QString("6 ") + Multilang("Hours Interval")
                                            << QString("12 ") + Multilang("Hours Interval")
                                            << QString("24 ") + Multilang("Hours Interval");

    m_sub_pageHeading = new TextLabel((SCALE_WIDTH(477)),
                                      (SCALE_HEIGHT(23)),
                                      SCALE_FONT(SUB_HEADING_FONT_SIZE),
                                      subHeadStr,
                                      this,
                                      HIGHLITED_FONT_COLOR,
                                      NORMAL_FONT_FAMILY,
                                      ALIGN_START_X_CENTRE_Y,
                                      0,
                                      false,
                                      SCALE_WIDTH(975),
                                      0);

    QMap<quint8, QString>  timezoneArrayList;
    for(quint8 index = 0; index < timezoneArray.length (); index++)
    {
        timezoneArrayList.insert (index,timezoneArray.at (index));
    }

	m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_LOCAL_TIME] = new OptionSelectButton(SCALE_WIDTH(40),
																SCALE_HEIGHT(45),
																RECT_WIDTH,
																RECT_HEIGHT,
																RADIO_BUTTON_INDEX,
																this, UP_LAYER,
																"Synchronize Camera Time With",
																wizardlabelstr[WIZARD_DTTIME_STG_SYNC_CAM_TIME_OPT_LOCAL_TIME],
																-1,
																WIZARD_DTTIME_STG_SYNC_CAM_TIME_OPT_LOCAL_TIME,
																true,
																NORMAL_FONT_SIZE,
																NORMAL_FONT_COLOR);
	connect (m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_LOCAL_TIME],
			 SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
			 this,
			 SLOT(slotSyncCameraTimeOptionButtonClicked(OPTION_STATE_TYPE_e,int)));

	m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_UTC_TIME] = new OptionSelectButton(m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_LOCAL_TIME]->x() + SCALE_WIDTH(650),
																m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_LOCAL_TIME]->y (),
																RECT_WIDTH,
																RECT_HEIGHT,
																RADIO_BUTTON_INDEX,
																wizardlabelstr[WIZARD_DTTIME_STG_SYNC_CAM_TIME_OPT_UTC_TIME],
																this,
																NO_LAYER,
																SCALE_WIDTH(0),
																MX_OPTION_TEXT_TYPE_SUFFIX,
																NORMAL_FONT_SIZE,
																WIZARD_DTTIME_STG_SYNC_CAM_TIME_OPT_UTC_TIME, -1,
																NORMAL_FONT_COLOR, true);
	connect (m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_UTC_TIME],
			 SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
			 this,
			 SLOT(slotSyncCameraTimeOptionButtonClicked(OPTION_STATE_TYPE_e,int)));

	m_UpdateTimeZoneOption = new OptionSelectButton(SCALE_WIDTH(40),
														m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_LOCAL_TIME]->y () + m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_LOCAL_TIME]->height (),
                                                        RECT_WIDTH,
                                                        RECT_HEIGHT,
                                                        CHECK_BUTTON_INDEX,
                                                        wizardlabelstr[WIZARD_DTTIME_STG_AUTO_UPDATE_TIMEZONE],
                                                        this,
                                                        UP_LAYER,
														-1,
														MX_OPTION_TEXT_TYPE_LABEL,
                                                        NORMAL_FONT_SIZE,
                                                        WIZARD_DTTIME_STG_AUTO_UPDATE_TIMEZONE, -1,
                                                        NORMAL_FONT_COLOR, true);

    m_timeZoneDropDownBox = new DropDown(m_UpdateTimeZoneOption->x(),
                                         m_UpdateTimeZoneOption->y() + m_UpdateTimeZoneOption->height(),
                                         RECT_WIDTH,
                                         RECT_HEIGHT,
                                         WIZARD_DTTIME_STG_TIME_ZONE,
                                         DROPDOWNBOX_SIZE_755,
                                         wizardlabelstr[WIZARD_DTTIME_STG_TIME_ZONE],
                                         timezoneArrayList,
                                         this, "", false, SCALE_WIDTH(80));

    dateCalTile = new CalendarTile(m_timeZoneDropDownBox->x (),
                                   m_timeZoneDropDownBox->y () + m_timeZoneDropDownBox->height (),
                                   RECT_WIDTH,
                                   RECT_HEIGHT,
                                   "",
                                   wizardlabelstr[WIZARD_DTTIME_STG_DATE],
                                   this, WIZARD_DTTIME_STG_DATE,
                                   true, 0,
                                   COMMON_LAYER);
    connect(dateCalTile,
            SIGNAL(sigDateChanged()),
            this,
            SLOT(slotDateChanged()));

    timeClockSpinBox = new ClockSpinbox(m_timeZoneDropDownBox->x (),
                                        dateCalTile->y () + dateCalTile->height (),
                                        RECT_WIDTH,
                                        RECT_HEIGHT,
                                        WIZARD_DTTIME_STG_TIME,
                                        CLK_SPINBOX_With_SEC,
                                        wizardlabelstr[WIZARD_DTTIME_STG_TIME],
                                        10, this, "", true,
                                        0,
                                        COMMON_LAYER);

    controlBtn = new ControlButton(SET_BUTTON_INDEX,
                                   m_timeZoneDropDownBox->x (),
                                   timeClockSpinBox->y () + timeClockSpinBox->height (),
                                   RECT_WIDTH,
                                   RECT_HEIGHT,
                                   this, COMMON_LAYER, -1,
                                   wizardlabelstr[WIZARD_DTTIME_STG_SET_BTN],
                                   true,
                                   WIZARD_DTTIME_STG_SET_BTN);
    connect(controlBtn,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotTimeDateSetBtnClicked(int)));

    autoSyncOption = new OptionSelectButton(controlBtn->x (),
                                            controlBtn->y () + controlBtn->height () + SCALE_HEIGHT(6),
                                            RECT_WIDTH,
                                            RECT_HEIGHT,
                                            CHECK_BUTTON_INDEX,
                                            wizardlabelstr[WIZARD_DTTIME_STG_CHECKBOX_AUTO],
                                            this,
                                            COMMON_LAYER,
                                            SCALE_WIDTH(20),
                                            MX_OPTION_TEXT_TYPE_SUFFIX,
                                            NORMAL_FONT_SIZE,
                                            WIZARD_DTTIME_STG_CHECKBOX_AUTO);
    connect(autoSyncOption,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotAutoSyncButtonClicked(OPTION_STATE_TYPE_e,int)));

    otherSerTextboxParam = new TextboxParam();
    otherSerTextboxParam->labelStr = wizardlabelstr[WIZARD_DTTIME_STG_NTP_SERVER];
    otherSerTextboxParam->maxChar = 40;
    otherSerTextboxParam->validation = QRegExp(asciiset1ValidationString);
    otherSerTextboxParam->isTotalBlankStrAllow = true;

    otherSerTextbox = new TextBox(autoSyncOption->x (),
                                  autoSyncOption->y () + autoSyncOption->height (),
                                  RECT_WIDTH,
                                  RECT_HEIGHT,
                                  WIZARD_DTTIME_STG_NTP_SERVER,
                                  TEXTBOX_LARGE,
                                  this, otherSerTextboxParam,
                                  COMMON_LAYER, false);

    QMap<quint8, QString>  updateIntStrMapList;
    for(quint8 index = 0; index < wizardUpdateIntStrList.length (); index++)
    {
        updateIntStrMapList.insert (index,wizardUpdateIntStrList.at (index));
    }
    updateIntervalDropDownBox = new DropDown(autoSyncOption->x (),
                                             otherSerTextbox->y ()+otherSerTextbox->height (),
                                             RECT_WIDTH,
                                             RECT_HEIGHT,
                                             WIZARD_DTTIME_STG_UPDATE_INTERVAL,
                                             DROPDOWNBOX_SIZE_200,
                                             wizardlabelstr[WIZARD_DTTIME_STG_UPDATE_INTERVAL],
                                             updateIntStrMapList,
                                             this, "", true, 0,
                                             COMMON_LAYER, false);

    QMap<quint8, QString>   lanStringList;
    lanStringList.clear ();
    m_languageDropDownBox = new DropDown(updateIntervalDropDownBox->x(),
                                         updateIntervalDropDownBox->y() + updateIntervalDropDownBox->height() +  SCALE_WIDTH(6),
                                         RECT_WIDTH,
                                         RECT_HEIGHT,
                                         WIZARD_DTTIME_STG_LANGUAGE,
                                         DROPDOWNBOX_SIZE_200,
                                         wizardlabelstr[WIZARD_DTTIME_STG_LANGUAGE],
                                         lanStringList,
                                         this, "", true, 0, COMMON_LAYER, true, 3);
    connect(m_languageDropDownBox,
            SIGNAL(sigValueChanged(QString,quint32)),
            this,
            SLOT(slotDropDownBoxValueChanged(QString,quint32)));

    m_infoPage = new InfoPage (0, 0,
                             SCALE_WIDTH(1145),
                             SCALE_HEIGHT(750),
                             MAX_INFO_PAGE_TYPE,
                             parentWidget());
    connect (m_infoPage,
             SIGNAL(sigInfoPageCnfgBtnClick(int)),
             this,
             SLOT(slotInfoPageBtnclick(int)));

    getConfig();
    getLanguage();
    this->show();
}

void DateAndTime::slotDropDownBoxValueChanged(QString string, quint32)
{
    WizardCommon::InfoPageImage();
    selectedLangStr = string;
    m_infoPage->raise();
    isErrRespInfoPage = false;
    m_infoPage->loadInfoPage (ValidationMessage::getValidationMessage(CHANGE_PREFEREED_LANGUAGE),true,false,"",CONFORMATION_BTN_YES,CONFORMATION_BTN_NO);
}

DateAndTime::~DateAndTime()
{
	if(IS_VALID_OBJ(m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_LOCAL_TIME]))
	{
		disconnect (m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_LOCAL_TIME],
					SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
					this,
					SLOT(slotSyncCameraTimeOptionButtonClicked(OPTION_STATE_TYPE_e,int)));
		DELETE_OBJ(m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_LOCAL_TIME]);
	}

	if(IS_VALID_OBJ(m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_UTC_TIME]))
	{
		disconnect (m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_UTC_TIME],
					SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
					this,
					SLOT(slotSyncCameraTimeOptionButtonClicked(OPTION_STATE_TYPE_e,int)));
		DELETE_OBJ(m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_UTC_TIME]);
	}

    DELETE_OBJ(m_UpdateTimeZoneOption);

    if(IS_VALID_OBJ(m_infoPage))
    {
        disconnect (m_infoPage,
                 SIGNAL(sigInfoPageCnfgBtnClick(int)),
                 this,
                SLOT(slotInfoPageBtnclick(int)));
        DELETE_OBJ(m_infoPage);
    }

    DELETE_OBJ(m_sub_pageHeading);
    DELETE_OBJ(m_timeZoneDropDownBox);

    if(IS_VALID_OBJ(dateCalTile))
    {
        disconnect(dateCalTile,
                   SIGNAL(sigDateChanged()),
                   this,
                   SLOT(slotDateChanged()));
        DELETE_OBJ(dateCalTile);
    }

    DELETE_OBJ(timeClockSpinBox);

    if(IS_VALID_OBJ(controlBtn))
    {
        disconnect(controlBtn,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotTimeDateSetBtnClicked(int)));
        DELETE_OBJ(controlBtn);
    }

    if(IS_VALID_OBJ(autoSyncOption))
    {
        disconnect(autoSyncOption,
                   SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                   this,
                   SLOT(slotAutoSyncButtonClicked(OPTION_STATE_TYPE_e,int)));
        DELETE_OBJ(autoSyncOption);
    }

    DELETE_OBJ(otherSerTextbox);
    DELETE_OBJ(otherSerTextboxParam);

    if(IS_VALID_OBJ(m_languageDropDownBox))
    {
        disconnect(m_languageDropDownBox,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotDropDownBoxValueChanged(QString,quint32)));
        DELETE_OBJ(m_languageDropDownBox);
    }

    DELETE_OBJ(updateIntervalDropDownBox);
    DELETE_OBJ(updateIntervalDropDownBox);
    DELETE_OBJ(payloadLib);

    WizardCommon:: UnloadProcessBar();
}

void DateAndTime::getLanguage()
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = GET_LANGUAGE;
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void DateAndTime::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    OPTION_STATE_TYPE_e state;

    if(deviceName != currDevName)
    {
        processBar->unloadProcessBar();
        return;
    }

    if (param->deviceStatus != CMD_SUCCESS)
    {
        WizardCommon::InfoPageImage();
        m_infoPage->raise();
        isErrRespInfoPage = true;
        m_infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
        processBar->unloadProcessBar();
        return;
    }

    switch(param->msgType)
    {
        case MSG_GET_CFG:
        {
            payloadLib->parsePayload(param->msgType, param->payload);

            if(payloadLib->getcnfgTableIndex () != DATETIME_TABLE_INDEX)
            {
                break;
            }

            m_timeZoneDropDownBox->setIndexofCurrElement(((payloadLib->getCnfgArrayAtIndex (FIELD_TIME_ZONE).toUInt ()) - 1));
            state = (OPTION_STATE_TYPE_e)(payloadLib->getCnfgArrayAtIndex (FIELD_NTP_AUTO_SYNC).toUInt ());
            serverIndex = (payloadLib->getCnfgArrayAtIndex(FIELD_NTP_SERVER).toUInt ());
            autoSyncOption->changeState(state);
            otherSerTextbox->setInputText((payloadLib->getCnfgArrayAtIndex (FIELD_OTHER_SERVER).toString ()));
            updateIntervalDropDownBox->setIndexofCurrElement(payloadLib->getCnfgArrayAtIndex (FIELD_UPDATE_INTERVAL).toUInt ());

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

            state = (OPTION_STATE_TYPE_e)(payloadLib->getCnfgArrayAtIndex (FIELD_AUTO_UPDATE_TIMEZONE).toUInt());
            m_UpdateTimeZoneOption->changeState(state);
            m_SyncCameraTimeOpt = (SYNC_CAM_TIME_OPTION_e)payloadLib->getCnfgArrayAtIndex (FIELD_SYNC_CAM_TIME_OPTION).toUInt ();
            updateSyncCameraTimeOptionButton();
            getDevDateTime();
            m_getConfig = true;
        }
        break;

        case MSG_SET_CFG:
        {
            m_deletePage = true;
            MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
        }
        break;

        case MSG_DEF_CFG:
        {
            getConfig();
        }
        break;

        case MSG_SET_CMD:
        {
            switch(param->cmdType)
            {
                case GET_DATE_TIME:
                {
                    payloadLib->parseDevCmdReply(true, param->payload);
                    updateDateTime(payloadLib->getCnfgArrayAtIndex(0).toString());
                }
                break;

                case SET_DATE_TIME:
                {
                    MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(DATE_TIME_SET_SUCCESS));
                }
                break;

                case GET_LANGUAGE:
                {
                    QMap<quint8, QString>   lanStringList;
                    lanStringList.clear();
                    payloadLib->parseDevCmdReply(true, param->payload);

                    for(quint8 tIndex = 0; tIndex < MAX_SYSTEM_LANGUAGE; tIndex++)
                    {
                        lanStringList.insert(tIndex, payloadLib->getCnfgArrayAtIndex(tIndex).toString());
                    }
                    m_languageDropDownBox->setNewList(lanStringList);
                    getUserPreferredLanguage();
                }
                break;

                case GET_USER_LANGUAGE:
                {
                    payloadLib->parseDevCmdReply(true, param->payload);
                    m_languageDropDownBox->setCurrValue(payloadLib->getCnfgArrayAtIndex(0).toString());
                }
                break;

                case SET_USER_LANGUAGE:
                default:
                {
                    /* Nothing to do */
                }
                break;
            }
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
    processBar->unloadProcessBar();
}

void DateAndTime::getConfig()
{
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

void DateAndTime::getDevDateTime()
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = GET_DATE_TIME;
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void DateAndTime::updateDateTime(QString dateTimeStr)
{
    m_dateStatus = true;
    DDMMYY_PARAM_t param;
    param.date  = dateTimeStr.mid(0, 2).toUInt();
    param.month = dateTimeStr.mid(2, 2).toUInt();
    param.year  = dateTimeStr.mid(4, 4).toUInt();
    dateCalTile->setDDMMYY (&param);
    timeClockSpinBox->assignValue(dateTimeStr.mid(8, 2), dateTimeStr.mid(10, 2), dateTimeStr.mid(12, 2));
}
void DateAndTime::slotTimeDateSetBtnClicked(int)
{
    // set device Date & time and show processing icon
    QString tempStr = "";
    quint32 hour, min, sec;

    DDMMYY_PARAM_t *dateParam = dateCalTile->getDDMMYY ();
    if (m_dateStatus == false)
    {
        WizardCommon:: InfoPageImage();
        m_infoPage->raise();
        isErrRespInfoPage = true;
        m_infoPage->loadInfoPage(ValidationMessage::getValidationMessage(INVALID_DATE_RANGE));
        return;
    }

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

    payloadLib->setCnfgArrayAtIndex(0, tempStr);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = SET_DATE_TIME;
    param->payload = payloadLib->createDevCmdPayload(1);
    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void DateAndTime::slotAutoSyncButtonClicked(OPTION_STATE_TYPE_e state,int)
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

void DateAndTime::saveConfig()
{
    if(m_getConfig == false)
    {
        return;
    }

    payloadLib->setCnfgArrayAtIndex (FIELD_TIME_ZONE, (m_timeZoneDropDownBox->getIndexofCurrElement () + 1));
    payloadLib->setCnfgArrayAtIndex (FIELD_NTP_AUTO_SYNC, autoSyncOption->getCurrentState ());
    payloadLib->setCnfgArrayAtIndex (FIELD_NTP_SERVER, serverIndex);
    payloadLib->setCnfgArrayAtIndex (FIELD_OTHER_SERVER, otherSerTextbox->getInputText ());
    payloadLib->setCnfgArrayAtIndex (FIELD_UPDATE_INTERVAL, updateIntervalDropDownBox->getIndexofCurrElement ());
    payloadLib->setCnfgArrayAtIndex (FIELD_AUTO_UPDATE_TIMEZONE, m_UpdateTimeZoneOption->getCurrentState ());
    payloadLib->setCnfgArrayAtIndex (FIELD_SYNC_CAM_TIME_OPTION, m_SyncCameraTimeOpt);

    QString payloadString = payloadLib->createDevCnfgPayload(MSG_SET_CFG,
                                                             DATETIME_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_TO_INDEX,
                                                             CNFG_FRM_FIELD,
                                                             MAX_DATE_TIME_FIELD_NO,
                                                             MAX_DATE_TIME_FIELD_NO);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CFG;
    param->payload = payloadString;
    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void DateAndTime::slotInfoPageBtnclick(int index)
{
    if (!isErrRespInfoPage)
    {
        emit sigLanguageCfgChanged(selectedLangStr);
        isErrRespInfoPage = true;
    }

    if ((false == isErrRespInfoPage) && (index == INFO_OK_BTN) && (m_infoPage->getText() != (ValidationMessage::getValidationMessage(INVALID_DATE_RANGE).toUtf8().constData())))
    {
        payloadLib->setCnfgArrayAtIndex(0, selectedLangStr);
        setUserPreferredLanguage();
    }

    WizardCommon::UnloadInfoPageImage();
}

void DateAndTime::slotSyncCameraTimeOptionButtonClicked(OPTION_STATE_TYPE_e state,int index)
{
	if(state == ON_STATE)
	{
		switch(index)
		{
            case WIZARD_DTTIME_STG_SYNC_CAM_TIME_OPT_LOCAL_TIME:
                m_SyncCameraTimeOpt = SYNC_CAM_TIME_OPTION_LOCAL_TIME;
                break;

            case WIZARD_DTTIME_STG_SYNC_CAM_TIME_OPT_UTC_TIME:
                m_SyncCameraTimeOpt = SYNC_CAM_TIME_OPTION_UTC_TIME;
                break;

            default:
                break;
		}

		/* Update Button State */
		updateSyncCameraTimeOptionButton();
	}
}

void DateAndTime::updateSyncCameraTimeOptionButton(void)
{
	switch(m_SyncCameraTimeOpt)
	{
        case SYNC_CAM_TIME_OPTION_LOCAL_TIME:
            /* Update Button State */
            m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_LOCAL_TIME]->changeState (ON_STATE);
            m_SyncCameraTimeOptButton[SYNC_CAM_TIME_OPTION_UTC_TIME]->changeState (OFF_STATE);
            /* On selecting LOCAL_TIME, Set Synchronize NVR Time Zone to ONVIF cameras to Disable */
            m_UpdateTimeZoneOption->changeState(OFF_STATE);
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

void DateAndTime::slotDateChanged(void)
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

void DateAndTime::getUserPreferredLanguage()
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = GET_USER_LANGUAGE;
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void DateAndTime::setUserPreferredLanguage()
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = SET_USER_LANGUAGE;
    param->payload = payloadLib->createDevCmdPayload(1);
    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}
