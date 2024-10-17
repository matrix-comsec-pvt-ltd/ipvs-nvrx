#include "BackupManagment.h"
#include "ValidationMessage.h"
#include "Controls/CnfgButton.h"

#define ELE_MAGIN   SCALE_WIDTH(25)

typedef enum
{
    BCK_ENB_CHECKBOX,
    BCK_SEL_TIME_INT_CHECKBOX,
    BCK_SEL_TIME_INT_SPINBOX,
    BCK_SEL_EVRDAY_CHEKBOX,
    BCK_SEL_EVRDAY_SPINBOX,
    BCK_SEL_WEEKTIME_CHECKBOX,
    BCK_SEL_WKDAY_SPINBOX,
    BCK_SEL_WKHOUR_SPINBOX,
    BCK_SEL_BCK_LOC,
    BCK_SEL_CAM_CTRL,
    BCK_SEL_MANUAL_BCK_CHK,
    BCK_SEL_DURATION_SPINBOX,
    BCK_SEL_MAN_BCK_LOC,
    BCK_MAN_SEARCH_START_DATE,
    BCK_MAN_SEARCH_START_TIME,
    BCK_MAN_SEARCH_END_DATE,
    BCK_MAN_SEARCH_END_TIME,
    BCK_MAN_START_BTN,
    BCK_SEL_MAN_CAM_CTRL,
    MAX_BCK_MNG_CTRL

}BCK_MNG_CTRL_e;

typedef enum
{
    SCH_BKP_ENABLE,
    SCH_BKP_MODE,
    SCH_BKP_EVERY_HOUR_MODE,
    SCH_BKP_EVERY_DAY_BKUP_HOUR,
    SCH_BKP_WEEKLY_BKUP_WEEK_DAYS,
    SCH_BKP_WEEKLY_BKUP_WEEK_HOUR,
    SCH_BKP_CAMERA_MASK_START,
    SCH_BKP_CAMERA_MASK_END = SCH_BKP_CAMERA_MASK_START + CAMERA_MASK_MAX - 1,
    SCH_BKP_BACKUP_LOCATION,
    SCH_BKP_CFG_FIELD_MAX

}SCH_BKP_CNFG_FIELDS_e;

typedef enum
{
    MAN_BKP_ENABLE,
    MAN_BKP_DURATION,
    MAN_BKP_CAMERA_MASK_START,
    MAN_BKP_CAMERA_MASK_END = MAN_BKP_CAMERA_MASK_START + CAMERA_MASK_MAX - 1,
    MAN_BKP_BACKUP_LOCATION,
    MAN_BKP_CFG_FIELD_MAX

}MAN_BKP_CNFG_FIELDS_e;

typedef enum
{
    BCK_ENBL_BACKUP_STR,
    BCK_ELE_HEADING,
    BCK_TIME_LBL,
    BCK_TIME_SUFIX,
    BCK_WEEKDAY_LBL,
    BCK_WEEKDAY_SUFIX,
    BCK_LOC_SPINBOX,
    BCK_SEL_CAM,
    BCK_CPYTO_HEADING,
    BCK_MANUAL_BCKUP_STR,
    BCK_DURATION_SPINBOX_STR,
    BCK_MANUAL_START_BTN,
    MAX_BCK_MNG_STR

}BCK_MNG_STR_e;

static const QString backupManagmentStrings[MAX_BCK_MNG_STR] =
{
    "Enable Scheduled Backup",
    "Backup Schedule",
    "Everyday at",
    "Hours",
    "Weekly on",
    "at",
    "Backup Location",
    "Select Cameras",
    "Schedule Backup",
    "Enable Manual Backup",
    "Duration",
    "Start"
};

static const QStringList weekdayList = QStringList ()
        << "Sun"
        << "Mon"
        << "Tue"
        << "Wed"
        << "Thu"
        << "Fri"
        << "Sat";

static QStringList durationList ;
static QStringList backupLocations;
static QStringList manualBackupLocations;
static QStringList timeIntList;

BackupManagment::BackupManagment(QString devName, QWidget *parent, DEV_TABLE_INFO_t *devTabInfo)
    : ConfigPageControl(devName, parent,MAX_BCK_MNG_CTRL,devTabInfo)
{
    createDefaultComponents();
    BackupManagment::getConfig();
    getDevDateTime();
    getCameraList();
    resetGeometryOfCnfgbuttonRow (SCALE_HEIGHT(45));
}

void BackupManagment::createDefaultComponents()
{
    INIT_OBJ(copyToCamera);
    INIT_OBJ(manualBackupCopyToCamera);
    INIT_OBJ(enableSchdBackup);
    INIT_OBJ(elementHeading);
    INIT_OBJ(manualBackupSelectCamera);
    INIT_OBJ(startDateCalender);
    INIT_OBJ(manualBackupSelectCamera);
    INIT_OBJ(startButton);
    INIT_OBJ(blank);
    INIT_OBJ(endTimeSpinbox);
    INIT_OBJ(startTimeSpinbox);
    INIT_OBJ(endDateCalender);
    memset(&manualBackupSelectedCameraNum, 0, sizeof(manualBackupSelectedCameraNum));
    memset(&selectedCameraNum, 0, sizeof(selectedCameraNum));

    durationList = QStringList ()
            << Multilang("Last") + QString(" 1 ") + Multilang("Hour")
            << Multilang("Last") + QString(" 2 ") + Multilang("Hour")
            << Multilang("Last") + QString(" 3 ") + Multilang("Hour")
            << Multilang("Last") + QString(" 4 ") + Multilang("Hour")
            << Multilang("Last") + QString(" 5 ") + Multilang("Hour")
            << Multilang("Last") + QString(" 6 ") + Multilang("Hour")
            << Multilang("Last") + QString(" 7 ") + Multilang("Hour")
            << Multilang("Last") + QString(" 8 ") + Multilang("Hour")
            << Multilang("Last") + QString(" 9 ") + Multilang("Hour")
            << Multilang("Last") + QString(" 10 ") + Multilang("Hour")
            << Multilang("Last") + QString(" 11 ") + Multilang("Hour")
            << Multilang("Last") + QString(" 12 ") + Multilang("Hour")
            << Multilang("Last") + QString(" 13 ") + Multilang("Hour")
            << Multilang("Last") + QString(" 14 ") + Multilang("Hour")
            << Multilang("Last") + QString(" 15 ") + Multilang("Hour")
            << Multilang("Last") + QString(" 16 ") + Multilang("Hour")
            << Multilang("Last") + QString(" 17 ") + Multilang("Hour")
            << Multilang("Last") + QString(" 18 ") + Multilang("Hour")
            << Multilang("Last") + QString(" 19 ") + Multilang("Hour")
            << Multilang("Last") + QString(" 20 ") + Multilang("Hour")
            << Multilang("Last") + QString(" 21 ") + Multilang("Hour")
            << Multilang("Last") + QString(" 22 ") + Multilang("Hour")
            << Multilang("Last") + QString(" 23 ") + Multilang("Hour")
            << Multilang("Last") + QString(" 24 ") + Multilang("Hour")
            << "Previous Day"
            << Multilang("Last") + QString(" 7 ") + Multilang("Days")
            << "All"
            << "Custom";

    backupLocations = QStringList ()
            << "USB Device"
            << Multilang("FTP Server") + QString(" 1")
            << Multilang("FTP Server") + QString(" 2")
            << Multilang("Network Drive") + QString(" 1")
            << Multilang("Network Drive") + QString(" 2");

    manualBackupLocations = QStringList ()
            << "USB Device"
            << Multilang("Network Drive") + QString(" 1")
            << Multilang("Network Drive") + QString(" 2");

    timeIntList = QStringList ()
            << Multilang("Last") + QString(" 1 ") + Multilang("Hour")
            << Multilang("Last") + QString(" 2 ") + Multilang("Hour")
            << Multilang("Last") + QString(" 3 ") + Multilang("Hour")
            << Multilang("Last") + QString(" 4 ") + Multilang("Hour")
            << Multilang("Last") + QString(" 6 ") + Multilang("Hour");

    enableSchdBackup = new OptionSelectButton((SCALE_WIDTH(PAGE_RIGHT_PANEL_WIDTH) - BGTILE_MEDIUM_SIZE_WIDTH)/2 + SCALE_WIDTH(10),
                                              (SCALE_HEIGHT(PAGE_RIGHT_PANEL_HEIGHT) - 16*BGTILE_HEIGHT)/2 - SCALE_HEIGHT(4),
                                              BGTILE_MEDIUM_SIZE_WIDTH,
                                              BGTILE_HEIGHT,
                                              CHECK_BUTTON_INDEX,
                                              backupManagmentStrings[BCK_ENBL_BACKUP_STR],
                                              this,
                                              COMMON_LAYER,
                                              ELE_MAGIN,
                                              MX_OPTION_TEXT_TYPE_SUFFIX,
                                              NORMAL_FONT_SIZE,
                                              BCK_ENB_CHECKBOX,
                                              true);
    m_elementList[BCK_ENB_CHECKBOX] = enableSchdBackup;
    connect (enableSchdBackup,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));
    connect (enableSchdBackup,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    elementHeading = new ElementHeading(enableSchdBackup->x (),
                                        enableSchdBackup->y () + BGTILE_HEIGHT,
                                        BGTILE_MEDIUM_SIZE_WIDTH,
                                        BGTILE_HEIGHT,
                                        backupManagmentStrings[BCK_ELE_HEADING],
                                        UP_LAYER,
                                        this,
                                        false,
                                        ELE_MAGIN, NORMAL_FONT_SIZE, true);

    for(quint8 index = 0; index < MAX_SCHD_MODE ; index++ )
    {
        backupSelection[index] = new OptionSelectButton (elementHeading->x (),
                                                         elementHeading->y () + (index + 1)*BGTILE_HEIGHT,
                                                         BGTILE_MEDIUM_SIZE_WIDTH,
                                                         BGTILE_HEIGHT,
                                                         RADIO_BUTTON_INDEX,
                                                         this,
                                                         index < 2 ? MIDDLE_TABLE_LAYER : BOTTOM_TABLE_LAYER,
                                                         "","",SCALE_WIDTH(30),
                                                         BCK_SEL_TIME_INT_CHECKBOX + 2*index,
                                                         false);
        m_elementList[BCK_SEL_TIME_INT_CHECKBOX + 2*index] = backupSelection[index];
        connect (backupSelection[index],
                 SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                 this,
                 SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));
        connect (backupSelection[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
    }

    QMap<quint8, QString>  timeIntMapList;
    for(quint8 index = 0; index < timeIntList.length (); index++)
    {
        timeIntMapList.insert (index,timeIntList.at (index));
    }

    timeIntervalDropDownBox = new DropDown(elementHeading->x () + SCALE_WIDTH(67),
                                           backupSelection[0]->y (),
                                           BGTILE_MEDIUM_SIZE_WIDTH,
                                           BGTILE_HEIGHT,
                                           BCK_SEL_TIME_INT_SPINBOX,
                                           DROPDOWNBOX_SIZE_200,
                                           "",
                                           timeIntMapList,
                                           this,"",false,
                                           0,
                                           NO_LAYER,false);
    m_elementList[BCK_SEL_TIME_INT_SPINBOX] = timeIntervalDropDownBox;
    connect (timeIntervalDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    timeHourList.clear ();
    for(quint8 index = 0; index < 24;index++)
    {
        timeHourList.insert (index, QString("%1").arg(index, 2, 10, QLatin1Char('0')));
    }

    timeHourSelectDropDownBox = new DropDown(elementHeading->x () + SCALE_WIDTH(70),
                                             backupSelection[1]->y (),
                                             BGTILE_MEDIUM_SIZE_WIDTH,
                                             BGTILE_HEIGHT,
                                             BCK_SEL_EVRDAY_SPINBOX,
                                             DROPDOWNBOX_SIZE_90,
                                             backupManagmentStrings[BCK_TIME_LBL],
                                             timeHourList,
                                             this,
                                             "",
                                             false,
                                             0,
                                             NO_LAYER,false);
    m_elementList[BCK_SEL_EVRDAY_SPINBOX] = timeHourSelectDropDownBox;
    connect (timeHourSelectDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    timeHourSuffix = new TextLabel(timeHourSelectDropDownBox->x () + timeHourSelectDropDownBox->width () + SCALE_WIDTH(10),
                                   timeHourSelectDropDownBox->y () + SCALE_HEIGHT(10),
                                   NORMAL_FONT_SIZE,
                                   backupManagmentStrings[BCK_TIME_SUFIX],
                                   this);

    weekdayLabel = new TextLabel(elementHeading->x () + SCALE_WIDTH(70),
                                 backupSelection[2]->y () + SCALE_HEIGHT(10),
                                 NORMAL_FONT_SIZE,
                                 backupManagmentStrings[BCK_WEEKDAY_LBL],
                                 this);

    QMap<quint8, QString>  weekdayMapList;
    for(quint8 index = 0; index < weekdayList.length (); index++)
    {
        weekdayMapList.insert (index,weekdayList.at (index));
    }

    weeklyDayDropDownBox = new DropDown (elementHeading->x () + SCALE_WIDTH(177),
                                         backupSelection[2]->y (),
                                         BGTILE_MEDIUM_SIZE_WIDTH,
                                         BGTILE_HEIGHT,
                                         BCK_SEL_WKDAY_SPINBOX,
                                         DROPDOWNBOX_SIZE_90,
                                         "",
                                         weekdayMapList,
                                         this,
                                         "",
                                         false,
                                         0,
                                         NO_LAYER,false);
    m_elementList[BCK_SEL_WKDAY_SPINBOX] = weeklyDayDropDownBox;
    connect (weeklyDayDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    weekdaySuffix = new TextLabel(weeklyDayDropDownBox->x () + weeklyDayDropDownBox->width () + SCALE_WIDTH(10),
                                  backupSelection[2]->y () + SCALE_HEIGHT(12),
                                  NORMAL_FONT_SIZE,
                                  backupManagmentStrings[BCK_WEEKDAY_SUFIX],
                                  this);

    weeklyTimeDropDownBox = new DropDown(weekdaySuffix->x () + weekdaySuffix->width () + SCALE_WIDTH(10),
                                         backupSelection[2]->y (),
                                         BGTILE_MEDIUM_SIZE_WIDTH,
                                         BGTILE_HEIGHT,
                                         BCK_SEL_WKHOUR_SPINBOX,
                                         DROPDOWNBOX_SIZE_90,
                                         "",
                                         timeHourList,
                                         this,
                                         "",
                                         false,
                                         0,
                                         NO_LAYER,false);
    m_elementList[BCK_SEL_WKHOUR_SPINBOX] = weeklyTimeDropDownBox;
    connect (weeklyTimeDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    weeklyTimeSuffix = new TextLabel(weeklyTimeDropDownBox->x () + weeklyTimeDropDownBox->width () + SCALE_WIDTH(10),
                                     weeklyTimeDropDownBox->y () + SCALE_HEIGHT(12),
                                     NORMAL_FONT_SIZE,
                                     backupManagmentStrings[BCK_TIME_SUFIX],
                                     this);

    QMap<quint8, QString>  backupLocationsMapList;
    for(quint8 index = 0; index < backupLocations.length (); index++)
    {
        backupLocationsMapList.insert (index,backupLocations.at (index));
    }

    backupLocationDropDownBox = new DropDown(elementHeading->x (),
                                             backupSelection[2]->y () + backupSelection[2]->height (),
                                             BGTILE_MEDIUM_SIZE_WIDTH,
                                             BGTILE_HEIGHT,
                                             BCK_SEL_BCK_LOC,
                                             DROPDOWNBOX_SIZE_200,
                                             backupManagmentStrings[BCK_LOC_SPINBOX],
                                             backupLocationsMapList,
                                             this,"",false,
                                             ELE_MAGIN,
                                             UP_LAYER,
                                             false);
    m_elementList[BCK_SEL_BCK_LOC] = backupLocationDropDownBox;
    connect (backupLocationDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    selectCamera = new ControlButton(BACKUP_BUTTON_INDEX,
                                     elementHeading->x (),
                                     backupLocationDropDownBox->y () + backupLocationDropDownBox->height (),
                                     BGTILE_MEDIUM_SIZE_WIDTH,
                                     BGTILE_HEIGHT,
                                     this,
                                     DOWN_LAYER,
                                     SCALE_WIDTH(300),
                                     backupManagmentStrings[BCK_SEL_CAM],
                                     false,
                                     BCK_SEL_CAM_CTRL);
    m_elementList[BCK_SEL_CAM_CTRL] = selectCamera;
    connect (selectCamera,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (selectCamera,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    manualBackUp = new OptionSelectButton(backupLocationDropDownBox->x (),
                                          selectCamera->y () +
                                          selectCamera->height () + SCALE_HEIGHT(10) - SCALE_HEIGHT(4),
                                          BGTILE_MEDIUM_SIZE_WIDTH,
                                          BGTILE_HEIGHT,
                                          CHECK_BUTTON_INDEX,
                                          backupManagmentStrings[BCK_MANUAL_BCKUP_STR],
                                          this,
                                          COMMON_LAYER,
                                          SCALE_WIDTH(20),
                                          MX_OPTION_TEXT_TYPE_SUFFIX,
                                          NORMAL_FONT_SIZE,
                                          BCK_SEL_MANUAL_BCK_CHK,
                                          true);
    m_elementList[BCK_SEL_MANUAL_BCK_CHK] = manualBackUp;
    connect (manualBackUp,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));
    connect (manualBackUp,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    QMap<quint8, QString>  durationMapList;
    for(quint8 index = 0; index < durationList.length (); index++)
    {
        durationMapList.insert (index,durationList.at (index));
    }

    durationDropDownBox = new DropDown(manualBackUp->x (),
                                       manualBackUp->y () + manualBackUp->height (),
                                       BGTILE_MEDIUM_SIZE_WIDTH,
                                       BGTILE_HEIGHT,
                                       BCK_SEL_DURATION_SPINBOX,
                                       DROPDOWNBOX_SIZE_200,
                                       backupManagmentStrings[BCK_DURATION_SPINBOX_STR],
                                       durationMapList,
                                       this, "", true,
                                       0, COMMON_LAYER,
                                       true, 5, false,
                                       false, 5, SCALE_WIDTH(60));
    m_elementList[BCK_SEL_DURATION_SPINBOX] = durationDropDownBox;
    connect (durationDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (durationDropDownBox,
             SIGNAL(sigValueChanged(QString, quint32)),
             this,
             SLOT(slotValueChanged(QString, quint32)));

    QMap<quint8, QString>  manualBackupLocationsMapList;
    for(quint8 index = 0; index < manualBackupLocations.length (); index++)
    {
        manualBackupLocationsMapList.insert (index,manualBackupLocations.at (index));
    }

    manualBackupLocationDropDownBox = new DropDown(elementHeading->x (),
                                                   durationDropDownBox->y () + durationDropDownBox->height (),
                                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                                   BGTILE_HEIGHT,
                                                   BCK_SEL_MAN_BCK_LOC,
                                                   DROPDOWNBOX_SIZE_200,
                                                   backupManagmentStrings[BCK_LOC_SPINBOX],
                                                   manualBackupLocationsMapList,
                                                   this,"", true, 0,
                                                   UP_LAYER, true, 8,
                                                   false, false, 5,
                                                   SCALE_WIDTH(60));
    m_elementList[BCK_SEL_MAN_BCK_LOC] = manualBackupLocationDropDownBox;
    connect (manualBackupLocationDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

	startDateCalender = new CalendarTile(manualBackupLocationDropDownBox->x (),
                                         manualBackupLocationDropDownBox->y () + manualBackupLocationDropDownBox->height (),
                                         BGTILE_MEDIUM_SIZE_WIDTH,
                                         BGTILE_HEIGHT,
                                         "", "",
                                         this, BCK_MAN_SEARCH_START_DATE, false,
                                         SCALE_WIDTH(10), UP_LAYER);
    m_elementList[BCK_MAN_SEARCH_START_DATE] = startDateCalender;
    connect(startDateCalender,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    endDateCalender = new CalendarTile(startDateCalender->x () + SCALE_WIDTH(265),
                                       startDateCalender->y () ,
                                       0,
                                       BGTILE_HEIGHT,
                                       "", "",
                                       this, BCK_MAN_SEARCH_END_DATE,
                                       false, 0, NO_LAYER);
    m_elementList[BCK_MAN_SEARCH_END_DATE] = endDateCalender;
    connect(endDateCalender,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    startTimeSpinbox = new ClockSpinbox(startDateCalender->x (),
                                        endDateCalender->y () + endDateCalender->height(),
                                        BGTILE_MEDIUM_SIZE_WIDTH ,
                                        BGTILE_HEIGHT ,
                                        BCK_MAN_SEARCH_START_TIME, CLK_SPINBOX_With_NO_SEC,
                                        "", 6,
                                        this, "", false,
                                        SCALE_WIDTH(20), UP_LAYER);
    m_elementList[BCK_MAN_SEARCH_START_TIME] = startTimeSpinbox;
    connect(startTimeSpinbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    endTimeSpinbox = new ClockSpinbox(endDateCalender->x () ,
                                      endDateCalender->y ()+ endDateCalender->height(),
                                      BGTILE_LARGE_SIZE_WIDTH + SCALE_WIDTH(126),
                                      BGTILE_HEIGHT, BCK_MAN_SEARCH_END_TIME,
                                      CLK_SPINBOX_With_NO_SEC,
                                      "", 6,
                                      this, "", false, SCALE_WIDTH(10), NO_LAYER);
    m_elementList[BCK_MAN_SEARCH_END_TIME] = endTimeSpinbox;
    connect(endTimeSpinbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    blank = new BgTile((SCALE_WIDTH(PAGE_RIGHT_PANEL_WIDTH) - BGTILE_MEDIUM_SIZE_WIDTH)/2 + SCALE_WIDTH(10),
                       endTimeSpinbox->y() + endTimeSpinbox->height(),
                       BGTILE_MEDIUM_SIZE_WIDTH,
                       BGTILE_HEIGHT+ SCALE_HEIGHT(6) ,
                       COMMON_LAYER,this,0);

    startButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                 this->width()/2,
                                 endTimeSpinbox->y() + endDateCalender->height()+ SCALE_HEIGHT(18) ,
                                 backupManagmentStrings[BCK_MANUAL_START_BTN],
                                 this,
                                 BCK_MAN_START_BTN,
                                 true);
    m_elementList[BCK_MAN_START_BTN] = startButton;
    connect(startButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotStartButtonClick(int)));

    manualBackupSelectCamera = new ControlButton(BACKUP_BUTTON_INDEX,
                                                 elementHeading->x (),
                                                 (startButton->y () +
                                                  blank->height ()),
                                                 BGTILE_MEDIUM_SIZE_WIDTH,
                                                 BGTILE_HEIGHT,
                                                 this,
                                                 DOWN_LAYER,
                                                 SCALE_WIDTH(300),
                                                 backupManagmentStrings[BCK_SEL_CAM],
                                                 false,
                                                 BCK_SEL_MAN_CAM_CTRL);
    m_elementList[BCK_SEL_MAN_CAM_CTRL] = manualBackupSelectCamera;
    connect (manualBackupSelectCamera,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (manualBackupSelectCamera,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));
}

BackupManagment::~BackupManagment()
{
    if(IS_VALID_OBJ(enableSchdBackup))
    {
        disconnect (enableSchdBackup,
                    SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                    this,
                    SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));
        disconnect (enableSchdBackup,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(enableSchdBackup);
    }

    DELETE_OBJ(elementHeading);

    for(quint8 index = 0; index < MAX_SCHD_MODE ; index++ )
    {
        if(IS_VALID_OBJ(backupSelection[index]))
        {
            disconnect (backupSelection[index],
                        SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                        this,
                        SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));
            disconnect (backupSelection[index],
                        SIGNAL(sigUpdateCurrentElement(int)),
                        this,
                        SLOT(slotUpdateCurrentElement(int)));
            DELETE_OBJ(backupSelection[index]);
        }
    }

    if(IS_VALID_OBJ(timeIntervalDropDownBox))
    {
        disconnect (timeIntervalDropDownBox,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(timeIntervalDropDownBox);
    }

    timeHourList.clear ();
    if(IS_VALID_OBJ(timeHourSelectDropDownBox))
    {
        disconnect (timeHourSelectDropDownBox,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(timeHourSelectDropDownBox);
    }

    DELETE_OBJ(timeHourSuffix);
    DELETE_OBJ(weekdayLabel);

    if(IS_VALID_OBJ(weeklyDayDropDownBox))
    {
        disconnect (weeklyDayDropDownBox,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(weeklyDayDropDownBox);
    }

    DELETE_OBJ(weekdaySuffix);

    if(IS_VALID_OBJ(weeklyTimeDropDownBox))
    {
        disconnect (weeklyTimeDropDownBox,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(weeklyTimeDropDownBox);
    }

    DELETE_OBJ(weeklyTimeSuffix);

    if(IS_VALID_OBJ(backupLocationDropDownBox))
    {
        disconnect (backupLocationDropDownBox,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(backupLocationDropDownBox);
    }

    if(IS_VALID_OBJ(selectCamera))
    {
        disconnect (selectCamera,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        disconnect (selectCamera,
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotButtonClick(int)));
        DELETE_OBJ(selectCamera);
    }

    if(IS_VALID_OBJ(manualBackUp))
    {
        disconnect (manualBackUp,
                    SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                    this,
                    SLOT(slotCheckBoxClicked(OPTION_STATE_TYPE_e,int)));
        disconnect (manualBackUp,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(manualBackUp);
    }

    if(IS_VALID_OBJ(durationDropDownBox))
    {
        disconnect (durationDropDownBox,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        disconnect (durationDropDownBox,
                 SIGNAL(sigValueChanged(QString, quint32)),
                 this,
                 SLOT(slotValueChanged(QString, quint32)));
        DELETE_OBJ(durationDropDownBox);
    }

    if(IS_VALID_OBJ(manualBackupLocationDropDownBox))
    {
        disconnect (manualBackupLocationDropDownBox,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(manualBackupLocationDropDownBox);
    }

    if(IS_VALID_OBJ(manualBackupSelectCamera))
    {
        disconnect (manualBackupSelectCamera,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        disconnect (manualBackupSelectCamera,
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotButtonClick(int)));
        DELETE_OBJ(manualBackupSelectCamera);
    }

    if ( IS_VALID_OBJ(startDateCalender))
    {
        disconnect(startDateCalender,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(startDateCalender);
    }

    if ( IS_VALID_OBJ(endDateCalender))
    {
        disconnect(endDateCalender,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));

        DELETE_OBJ(endDateCalender);
    }

    if(IS_VALID_OBJ(startTimeSpinbox))
    {
        disconnect(startTimeSpinbox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(startTimeSpinbox);
    }

    if(IS_VALID_OBJ(endTimeSpinbox))
    {
        disconnect(endTimeSpinbox,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(endTimeSpinbox);
    }

    DELETE_OBJ(blank);

    if(IS_VALID_OBJ(startButton))
    {
        disconnect(startButton,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonClick(int)));
        DELETE_OBJ(startButton);
    }
}

void BackupManagment::getCameraList()
{
    cameraList.clear();
    for(quint8 index = 0; index < devTableInfo->totalCams; index++)
    {
        cameraList.insert(index, QString("%1%2%3").arg(index + 1).arg(" : ").arg(applController->GetCameraNameOfDevice(currDevName, index)));
    }
}

void BackupManagment::createPayload(REQ_MSG_ID_e msgType)
{
    QString payloadString = payloadLib->createDevCnfgPayload(msgType,
                                                             SCHEDULE_BACKUP_MANAGMENT_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             SCH_BKP_CFG_FIELD_MAX,
                                                             SCH_BKP_CFG_FIELD_MAX);

    if ((msgType != MSG_SET_CFG) || (isUserChangeConfig()))
    {
        payloadString = payloadLib->createDevCnfgPayload(msgType,
                                                         MANUAL_BACKUP_MANAGMENT_TABLE_INDEX,
                                                         CNFG_FRM_INDEX,
                                                         CNFG_FRM_INDEX,
                                                         CNFG_FRM_INDEX,
                                                         MAN_BKP_CFG_FIELD_MAX,
                                                         MAN_BKP_CFG_FIELD_MAX,
                                                         payloadString,
                                                         SCH_BKP_CFG_FIELD_MAX);
    }

    DevCommParam* param = new DevCommParam();
    param->msgType = msgType;
    param->payload = payloadString;
    processBar->loadProcessBar ();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void BackupManagment::defaultConfig()
{
    createPayload(MSG_DEF_CFG);
}

void BackupManagment::getConfig()
{
    createPayload(MSG_GET_CFG);
}

void BackupManagment::saveConfig()
{
    if(isUserChangeConfig())
    {
        infoPage->unloadInfoPage();
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(MAN_BACKUP_SAVE_VALIDATION), true, false, "", CONFORMATION_BTN_YES, CONFORMATION_BTN_NO);
    }
    else
    {
        saveConfigFields();
    }
}

void BackupManagment::saveConfigFields()
{
    quint8 temp;
    QString tempStr;

    temp = enableSchdBackup->getCurrentState();
    payloadLib->setCnfgArrayAtIndex(SCH_BKP_ENABLE, temp);

    temp = (backupSelection[0]->getCurrentState() == ON_STATE) ? 0 : ((backupSelection[1]->getCurrentState() == ON_STATE) ? 1 : 2);
    payloadLib->setCnfgArrayAtIndex(SCH_BKP_MODE,temp);

    temp = timeIntervalDropDownBox->getIndexofCurrElement();
    tempStr = (temp < 10) ? QString("%1%2").arg(0).arg(temp) : QString("%1").arg(temp);
    payloadLib->setCnfgArrayAtIndex(SCH_BKP_EVERY_HOUR_MODE,tempStr);

    temp = timeHourSelectDropDownBox->getIndexofCurrElement();
    tempStr = (temp < 10) ? QString("%1%2").arg(0).arg(temp) : QString("%1").arg(temp);
    payloadLib->setCnfgArrayAtIndex(SCH_BKP_EVERY_DAY_BKUP_HOUR,tempStr);

    temp = weeklyDayDropDownBox->getIndexofCurrElement();
    payloadLib->setCnfgArrayAtIndex(SCH_BKP_WEEKLY_BKUP_WEEK_DAYS,temp);

    temp = weeklyTimeDropDownBox->getIndexofCurrElement();
    tempStr = (temp < 10) ? QString("%1%2").arg(0).arg(temp) : QString("%1").arg(temp);
    payloadLib->setCnfgArrayAtIndex(SCH_BKP_WEEKLY_BKUP_WEEK_HOUR,tempStr);

    temp = backupLocationDropDownBox->getIndexofCurrElement();
    payloadLib->setCnfgArrayAtIndex(SCH_BKP_BACKUP_LOCATION,temp);

    temp = manualBackUp->getCurrentState();
    payloadLib->setCnfgArrayAtIndex(SCH_BKP_CFG_FIELD_MAX + MAN_BKP_ENABLE,temp);

    temp = durationDropDownBox->getIndexofCurrElement();
    payloadLib->setCnfgArrayAtIndex(SCH_BKP_CFG_FIELD_MAX + MAN_BKP_DURATION,temp);

    temp = manualBackupLocationDropDownBox->getIndexofCurrElement();
    payloadLib->setCnfgArrayAtIndex(SCH_BKP_CFG_FIELD_MAX + MAN_BKP_BACKUP_LOCATION,temp);

    for(quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
    {
        payloadLib->setCnfgArrayAtIndex((SCH_BKP_CAMERA_MASK_START + maskIdx),selectedCameraNum.bitMask[maskIdx]);
        payloadLib->setCnfgArrayAtIndex(SCH_BKP_CFG_FIELD_MAX + (MAN_BKP_CAMERA_MASK_START + maskIdx), manualBackupSelectedCameraNum.bitMask[maskIdx]);
    }
    createPayload (MSG_SET_CFG);
}

void BackupManagment::processDeviceResponse (DevCommParam *param, QString deviceName)
{
    if (deviceName != currDevName)
    {
        return;
    }

    if (param->deviceStatus != CMD_SUCCESS)
    {
        processBar->unloadProcessBar();
        infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus), false, false, "", CONFORMATION_BTN_OK, "");
        return;
    }

    switch(param->msgType)
    {
        case MSG_GET_CFG:
        {
            processBar->unloadProcessBar();
            payloadLib->parsePayload (param->msgType, param->payload);
            if((payloadLib->getcnfgTableIndex(0) != SCHEDULE_BACKUP_MANAGMENT_TABLE_INDEX)
                    || (payloadLib->getcnfgTableIndex(1) != MANUAL_BACKUP_MANAGMENT_TABLE_INDEX))
            {
                break;
            }

            quint8 temp;
            for(quint8 index = 0; index < MAX_SCHD_MODE; index++)
            {
                backupSelection[index]->changeState(OFF_STATE);
            }

            temp = payloadLib->getCnfgArrayAtIndex(SCH_BKP_MODE).toUInt();
            backupSelection[temp]->changeState(ON_STATE);

            temp = payloadLib->getCnfgArrayAtIndex(SCH_BKP_EVERY_HOUR_MODE).toUInt();
            timeIntervalDropDownBox->setIndexofCurrElement(temp);

            temp = payloadLib->getCnfgArrayAtIndex(SCH_BKP_EVERY_DAY_BKUP_HOUR).toUInt();
            timeHourSelectDropDownBox->setIndexofCurrElement(temp);

            temp = payloadLib->getCnfgArrayAtIndex(SCH_BKP_WEEKLY_BKUP_WEEK_DAYS).toUInt();
            weeklyDayDropDownBox->setIndexofCurrElement(temp);

            temp = payloadLib->getCnfgArrayAtIndex(SCH_BKP_WEEKLY_BKUP_WEEK_HOUR).toUInt();
            weeklyTimeDropDownBox->setIndexofCurrElement(temp);

            temp = payloadLib->getCnfgArrayAtIndex(SCH_BKP_BACKUP_LOCATION).toUInt();
            backupLocationDropDownBox->setIndexofCurrElement(temp);

            for(quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
            {
                selectedCameraNum.bitMask[maskIdx] = payloadLib->getCnfgArrayAtIndex(SCH_BKP_CAMERA_MASK_START + maskIdx).toULongLong();
                configCopy.camera.bitMask[maskIdx] = payloadLib->getCnfgArrayAtIndex(SCH_BKP_CFG_FIELD_MAX + (MAN_BKP_CAMERA_MASK_START + maskIdx)).toULongLong();
                manualBackupSelectedCameraNum.bitMask[maskIdx] = configCopy.camera.bitMask[maskIdx];
            }

            temp = payloadLib->getCnfgArrayAtIndex(SCH_BKP_ENABLE).toUInt();
            enableSchdBackup->changeState(temp == 1 ? ON_STATE : OFF_STATE);

            enableControls(temp);

            configCopy.manBackupCheckBox = payloadLib->getCnfgArrayAtIndex(SCH_BKP_CFG_FIELD_MAX + MAN_BKP_ENABLE).toUInt();
            configCopy.duration = payloadLib->getCnfgArrayAtIndex(SCH_BKP_CFG_FIELD_MAX + MAN_BKP_DURATION).toUInt();
            configCopy.backupLocation = payloadLib->getCnfgArrayAtIndex(SCH_BKP_CFG_FIELD_MAX + MAN_BKP_BACKUP_LOCATION).toUInt();

            temp = configCopy.manBackupCheckBox;
            manualBackUp->changeState((OPTION_STATE_TYPE_e)configCopy.manBackupCheckBox);
            durationDropDownBox->setIsEnabled(configCopy.manBackupCheckBox);
            manualBackupSelectCamera->setIsEnabled(configCopy.manBackupCheckBox);
            startDateCalender->setIsEnabled(configCopy.manBackupCheckBox);
            endDateCalender->setIsEnabled(configCopy.manBackupCheckBox);
            startTimeSpinbox->setIsEnabled(configCopy.manBackupCheckBox);
            endTimeSpinbox->setIsEnabled(configCopy.manBackupCheckBox);
            startButton->setIsEnabled(configCopy.manBackupCheckBox);

            durationDropDownBox->setIndexofCurrElement(configCopy.duration);
            if (configCopy.duration == 27)
            {
                startDateCalender->setIsEnabled(configCopy.manBackupCheckBox);
                endDateCalender->setIsEnabled(configCopy.manBackupCheckBox);
                startTimeSpinbox->setIsEnabled(configCopy.manBackupCheckBox);
                endTimeSpinbox->setIsEnabled(configCopy.manBackupCheckBox);
            }
            else
            {
                startDateCalender->setIsEnabled(false);
                endDateCalender->setIsEnabled(false);
                startTimeSpinbox->setIsEnabled(false);
                endTimeSpinbox->setIsEnabled(false);
            }

            manualBackupLocationDropDownBox->setIndexofCurrElement(configCopy.backupLocation);
        }
        break;

        case MSG_SET_CFG:
        {
            processBar->unloadProcessBar();
            MessageBanner::addMessageInBanner (ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
            getConfig();
        }
        break;

        case MSG_DEF_CFG:
        {
            processBar->unloadProcessBar();
            getConfig();
        }
        break;

        case MSG_SET_CMD:
        {
            DDMMYY_PARAM_t dateTimeParam;
            switch(param->cmdType)
            {
                case GET_DATE_TIME:
                {
                    processBar->unloadProcessBar();
                    payloadLib->parseDevCmdReply(true, param->payload);

                    QDateTime dt = QDateTime::fromString (payloadLib->getCnfgArrayAtIndex(0).toString(), "ddMMyyyyHHmmss");
                    QDate date = dt.date ();
                    QTime time = dt.time ();

                    dateTimeParam.date  = date.day ();
                    dateTimeParam.month = date.month ();
                    dateTimeParam.year  = date.year ();
                    endDateCalender->setDDMMYY (&dateTimeParam);
                    endTimeSpinbox->assignValue (time.hour (), time.minute ());

                    // show start time 1 Hr Before
                    dt = dt.addSecs (-3600);
                    date = dt.date ();
                    time = dt.time ();
                    dateTimeParam.date  = date.day ();
                    dateTimeParam.month = date.month ();
                    dateTimeParam.year  = date.year ();
                    startDateCalender->setDDMMYY (&dateTimeParam);
                    startTimeSpinbox->assignValue (time.hour (), time.minute ());
                }
                break;

                case MAN_BACKUP:
                {
                    processBar->unloadProcessBar ();
                }
                break;

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
}

void BackupManagment::enableControls(quint32 state)
{
    for(quint8 index = 0; index < MAX_SCHD_MODE ; index++)
    {
        backupSelection[index]->setIsEnabled(state);
    }

    if(backupSelection[0]->getCurrentState())
    {
        timeIntervalDropDownBox->setIsEnabled(state);
    }

    if(backupSelection[1]->getCurrentState())
    {
        timeHourSelectDropDownBox->setIsEnabled(state);
    }

    if(backupSelection[2]->getCurrentState())
    {
        weeklyDayDropDownBox->setIsEnabled(state);
        weeklyTimeDropDownBox->setIsEnabled(state);
    }

    backupLocationDropDownBox->setIsEnabled(state);
    selectCamera->setIsEnabled(state);
}

void BackupManagment::slotCheckBoxClicked (OPTION_STATE_TYPE_e state, int index)
{
    switch(index)
    {
        case BCK_ENB_CHECKBOX:
        {
            enableControls(state);
        }
        break;

        case BCK_SEL_TIME_INT_CHECKBOX:
        {
            backupSelection[1]->changeState(OFF_STATE);
            backupSelection[2]->changeState(OFF_STATE);
            timeIntervalDropDownBox->setIsEnabled(true);
            timeHourSelectDropDownBox->setIsEnabled(false);
            weeklyDayDropDownBox->setIsEnabled(false);
            weeklyTimeDropDownBox->setIsEnabled(false);
        }
        break;

        case BCK_SEL_EVRDAY_CHEKBOX:
        {
            backupSelection[0]->changeState(OFF_STATE);
            backupSelection[2]->changeState(OFF_STATE);
            timeIntervalDropDownBox->setIsEnabled(false);
            timeHourSelectDropDownBox->setIsEnabled(true);
            weeklyDayDropDownBox->setIsEnabled(false);
            weeklyTimeDropDownBox->setIsEnabled(false);
        }
        break;

        case BCK_SEL_WEEKTIME_CHECKBOX:
        {
            backupSelection[0]->changeState(OFF_STATE);
            backupSelection[1]->changeState(OFF_STATE);
            timeIntervalDropDownBox->setIsEnabled(false);
            timeHourSelectDropDownBox->setIsEnabled(false);
            weeklyDayDropDownBox->setIsEnabled(true);
            weeklyTimeDropDownBox->setIsEnabled(true);
        }
        break;

        case BCK_SEL_MANUAL_BCK_CHK:
        {
            durationDropDownBox->setIsEnabled(state == ON_STATE ?  true : false);
            manualBackupSelectCamera->setIsEnabled(state == ON_STATE ?  true : false);
            startButton->setIsEnabled(state == ON_STATE ?  true : false);
            quint8 localDropdnIndex = durationDropDownBox->getIndexofCurrElement();

            if(localDropdnIndex==27)
            {
                startDateCalender->setIsEnabled(state == ON_STATE ?  true : false);
                endDateCalender->setIsEnabled(state == ON_STATE ?  true : false);
                startTimeSpinbox->setIsEnabled(state == ON_STATE ?  true : false);
                endTimeSpinbox->setIsEnabled(state == ON_STATE ?  true : false);
            }
            else
            {
                startDateCalender->setIsEnabled(false);
                endDateCalender->setIsEnabled(false);
                startTimeSpinbox->setIsEnabled(false);
                endTimeSpinbox->setIsEnabled(false);
            }
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void BackupManagment::slotButtonClick (int index)
{
    switch(index)
    {
        case BCK_SEL_CAM_CTRL:
        {
            if (IS_VALID_OBJ(copyToCamera))
            {
                break;
            }

            /* PARASOFT: Memory Deallocated in slot SubObjectDelete */
            copyToCamera = new CopyToCamera(cameraList,
                                            selectedCameraNum,
                                            parentWidget (),
                                            "Schedule Backup");
            connect (copyToCamera,
                     SIGNAL(sigDeleteObject(quint8)),
                     this,
                     SLOT(slotSubObjectDelete(quint8)));
        }
        break;

        case BCK_SEL_MAN_CAM_CTRL:
        {
            if (IS_VALID_OBJ(manualBackupCopyToCamera))
            {
                break;
            }

            /* PARASOFT: Memory Deallocated in slot SubObjectDelete */
            manualBackupCopyToCamera = new CopyToCamera(cameraList,
                                                        manualBackupSelectedCameraNum,
                                                        parentWidget (),
                                                        "Manual Backup");
            connect (manualBackupCopyToCamera,
                     SIGNAL(sigDeleteObject(quint8)),
                     this,
                     SLOT(slotSubObjectDelete(quint8)));
        }
        break;
    }
}

void BackupManagment::slotSubObjectDelete (quint8)
{
    if(copyToCamera != NULL)
    {
        disconnect (copyToCamera,
                    SIGNAL(sigDeleteObject(quint8)),
                    this,
                    SLOT(slotSubObjectDelete(quint8)));
        DELETE_OBJ(copyToCamera);
    }

    if(manualBackupCopyToCamera != NULL)
    {
        disconnect (manualBackupCopyToCamera,
                    SIGNAL(sigDeleteObject(quint8)),
                    this,
                    SLOT(slotSubObjectDelete(quint8)));
        DELETE_OBJ(manualBackupCopyToCamera);
    }

    m_elementList[m_currentElement]->forceActiveFocus ();
}

void BackupManagment::slotValueChanged(QString value, quint32)
{
    if("Custom" == value)
    {
        startDateCalender->setIsEnabled(true);
        endDateCalender->setIsEnabled(true);
        startTimeSpinbox->setIsEnabled(true);
        endTimeSpinbox->setIsEnabled(true);
    }
    else
    {
        startDateCalender->setIsEnabled(false);
        endDateCalender->setIsEnabled(false);
        startTimeSpinbox->setIsEnabled(false);
        endTimeSpinbox->setIsEnabled(false);
    }
}

void BackupManagment::getDevDateTime()
{
    DevCommParam *param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = GET_DATE_TIME;
    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

bool BackupManagment::isUserChangeConfig()
{
    if (configCopy.manBackupCheckBox != manualBackUp->getCurrentState())
    {
        return true;
    }

    if(configCopy.duration != durationDropDownBox->getIndexofCurrElement())
    {
        return true;
    }

    if(configCopy.backupLocation != manualBackupLocationDropDownBox->getIndexofCurrElement())
    {
        return true;
    }

    for(quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
    {
        if(configCopy.camera.bitMask[maskIdx] != manualBackupSelectedCameraNum.bitMask[maskIdx])
        {
            return true;
        }
    }

    return false;
}

void BackupManagment::handleInfoPageMessage(int index)
{
    if(index == INFO_OK_BTN)
    {
        if(infoPage->getText() == (ValidationMessage::getValidationMessage(MAN_BACKUP_VALIDATION))
                ||infoPage->getText() == (ValidationMessage::getValidationMessage(MAN_BACKUP_SAVE_VALIDATION)))
        {
            saveConfigFields();
        }
    }
    else
    {
        if(infoPage->getText() == (ValidationMessage::getValidationMessage(MAN_BACKUP_VALIDATION))
                ||infoPage->getText() == (ValidationMessage::getValidationMessage(MAN_BACKUP_SAVE_VALIDATION)))
        {
            getConfig();
        }
    }
}

void BackupManagment::slotStartButtonClick(int index)
{
    switch(index)
    {
        case BCK_MAN_START_BTN:
        {
            if(isUserChangeConfig())
            {
                infoPage->unloadInfoPage();
                infoPage->loadInfoPage(ValidationMessage::getValidationMessage(MAN_BACKUP_VALIDATION),true,false,"",CONFORMATION_BTN_YES,CONFORMATION_BTN_NO);
            }
            else
            {
                startBackup();
            }
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void BackupManagment::startBackup()
{
    quint64 startTime=0,endTime=0;

    DDMMYY_PARAM_t * startDate;
    quint32 startHour=0,startMin=0,startSec=0;
    QDate Date;
    QTime Time;
    QDateTime startDateTime, endDateTime;

    if(durationDropDownBox->getCurrValue() == "Custom")
    {
        startDate = startDateCalender->getDDMMYY();
        startTimeSpinbox->currentValue(startHour,startMin,startSec);
        startTime=(startTime)+startDate->date;
        startTime=(startTime*100)+startDate->month;
        startTime=(startTime*10000)+startDate->year;
        startTime=(startTime*100)+startHour;
        startTime=(startTime*100)+startMin;
        startTime=(startTime*100)+startSec;

        Date.setDate (startDate->year, startDate->month, startDate->date);
        Time.setHMS (startHour, startMin, startSec);

        startDateTime.setDate (Date);
        startDateTime.setTime (Time);

        DDMMYY_PARAM_t * endDate = endDateCalender->getDDMMYY();
        quint32 endHour=0,endMin=0,endSec=0;
        endTimeSpinbox->currentValue(endHour,endMin,endSec);

        endTime=(endTime)+endDate->date;
        endTime=(endTime*100)+endDate->month;
        endTime=(endTime*10000)+endDate->year;
        endTime=(endTime*100)+endHour;
        endTime=(endTime*100)+endMin;
        endTime=(endTime*100)+endSec;

        Date.setDate (endDate->year, endDate->month, endDate->date);
        Time.setHMS (endHour, endMin, endSec);
        endDateTime.setDate (Date);
        endDateTime.setTime (Time);
    }
    else
    {
        startTime = 0000000;
        endTime = 0000000;
    }

    if((startDateTime < endDateTime) || ((startTime == 0000000 ) && (endTime == 0000000)))
    {
        payloadLib->setCnfgArrayAtIndex(0,startTime);
        payloadLib->setCnfgArrayAtIndex(1,endTime);
        QString payloadString = payloadLib->createDevCmdPayload(2);

        DevCommParam* param = new DevCommParam();
        param->msgType = MSG_SET_CMD;
        param->cmdType = MAN_BACKUP;
        param->payload = payloadString;

        processBar->loadProcessBar ();
        applController->processActivity(currDevName, DEVICE_COMM, param);
    }
    else
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(START_END_DATE_ERR_MSG),false,false,"");
    }
}
