#include "Recording.h"
#include "ValidationMessage.h"

#define RECORDING_FROM_FIELD                1
#define MAX_REC_ENB_SCH_FIELD_NO            (REC_CPY_TO_CAMERA_END - REC_ENB_SCHD_REC + 1)
#define MAX_REC_SCH_REC_FIELD_NO            (REC_SCH_CPY_TO_CAMERA_END - REC_SCH_ENTIRE_DAY + 1)
#define MAX_REC_ALM_REC_FIELD_NO            (REC_ALM_CPY_TO_CAMERA_END - REC_PRE_RECORD_TIME + 1)
#define MAX_REC_COS_REC_FIELD_NO            (REC_COSEC_CPY_TO_CAMERA_END - REC_COSEC_ENB_REC + 1)
#define MAX_REC_MANUAL_REC_FIELD_NO         (REC_MANUAL_CPY_TO_CAMERA_END - REC_MANUAL_ENB_REC + 1)

#define MAX_REC_SCH_TOTAL_FEILD             MAX_WEEKDAYS*MAX_REC_SCH_REC_FIELD_NO
#define REC_ALARM_START_FEILD               (7*MAX_REC_SCH_REC_FIELD_NO)+MAX_REC_ENB_SCH_FIELD_NO
#define REC_COSEC_START_FEILD               MAX_REC_ALM_REC_FIELD_NO + REC_ALARM_START_FEILD
#define REC_MANUAL_REC_START_FEILD          MAX_REC_COS_REC_FIELD_NO + REC_COSEC_START_FEILD

typedef enum{
    REC_CAMERANAME_SPINBOX_STRING,
    REC_CPY_CAMERA_BTN_STRING,
    REC_ALARM_REC_STRING,
    REC_PRERECORD_SUFFIX_STRING,
    REC_PRERECORD_LABEL_STRING,
    REC_POSTRECORD_LABEL_STRING,
    REC_POSTRECORD_SUFFIX_STRING,
    REC_COSEC_HEADING_STRING,
    REC_ENBL_PRE_REC_STRING,
    REC_MAN_REC_STRING,
    REC_SCHD_REC_STRING,
    REC_SCHD_WEEK_STRING,
    REC_SET_STRING = REC_SCHD_WEEK_STRING + MAX_WEEKDAYS,
    REC_COPY_TO_CAM_HEADING,
    MAX_RECODING_STRING
}RECORDING_STRING_e;

static QString recordingStrings[MAX_RECODING_STRING]= {
    "Camera",
    "Copy to Camera",
    "Alarm Recording",
    "(0 - 30 sec)",
    "Pre Record",
    "Post Record",
    "(10 - 300 sec)",
    "COSEC Recording",
    "Enable Pre Record",
    "Manual Recording",
    "Scheduled Recording",
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
    "Set",
    "Recording"
};

typedef enum{
    REC_ENB_SCHD_REC = 0,
    REC_CPY_TO_CAMERA_START,
    REC_CPY_TO_CAMERA_END = (REC_CPY_TO_CAMERA_START + CAMERA_MASK_MAX - 1),
    REC_SCH_ENTIRE_DAY ,
    REC_SCH_START_TIME1,
    REC_SCH_STOP_TIME1,
    REC_SCH_START_TIME2,
    REC_SCH_STOP_TIME2,
    REC_SCH_START_TIME3,
    REC_SCH_STOP_TIME3,
    REC_SCH_START_TIME4,
    REC_SCH_STOP_TIME4,
    REC_SCH_START_TIME5,
    REC_SCH_STOP_TIME5,
    REC_SCH_START_TIME6,
    REC_SCH_STOP_TIME6,
    REC_SCH_CPY_TO_CAMERA_START,
    REC_SCH_CPY_TO_CAMERA_END = (REC_SCH_CPY_TO_CAMERA_START + CAMERA_MASK_MAX - 1),
    REC_PRE_RECORD_TIME = (MAX_REC_SCH_TOTAL_FEILD + MAX_REC_ENB_SCH_FIELD_NO),
    REC_POST_RECORD_TIME,
    REC_ALM_CPY_TO_CAMERA_START,
    REC_ALM_CPY_TO_CAMERA_END = (REC_ALM_CPY_TO_CAMERA_START + CAMERA_MASK_MAX - 1),
    REC_COSEC_ENB_REC,
    REC_COSEC_PRE_RECORD_VALUE,
    REC_COSEC_CPY_TO_CAMERA_START,
    REC_COSEC_CPY_TO_CAMERA_END = (REC_COSEC_CPY_TO_CAMERA_START + CAMERA_MASK_MAX - 1),
    REC_MANUAL_ENB_REC,
    REC_MANUAL_CPY_TO_CAMERA_START,
    REC_MANUAL_CPY_TO_CAMERA_END = (REC_MANUAL_CPY_TO_CAMERA_START + CAMERA_MASK_MAX - 1),
    MAX_REC_REC_FIELD_NO
}REC_FIELD_NO_e;



Recording::Recording(QString deviceName, QWidget* parent,DEV_TABLE_INFO_t *devTabInfo)
    : ConfigPageControl(deviceName, parent,MAX_RECORDING_CONTROL,devTabInfo), currentDaySelect(0)
{
    copytoCamera = NULL;
    setSchedule = NULL;
    copyToWeekdaysFields = 0;
    createDefaultComponents();
    Recording::getConfig ();
}

void Recording::fillCameraList ()
{
    QString tempStr;
    cameraList.clear();

    for(quint8 index = 0; index < devTableInfo->totalCams; index++)
    {
        tempStr = applController->GetCameraNameOfDevice(currDevName,index);
        if(((index + 1) < 10) && (devTableInfo->totalCams > 10))
        {
            cameraList.insert(index, QString(" %1%2%3").arg(index + 1)
                              .arg(" : ").arg (tempStr));
        }
        else
        {
            cameraList.insert(index, QString("%1%2%3").arg(index + 1)
                              .arg(" : ").arg (tempStr));
        }
    }

    cameraNameSpinbox->setNewList (cameraList);
    cameraNameSpinbox->setIndexofCurrElement (currentCameraIndex - 1);
}

void Recording::createDefaultComponents()
{
    quint8 leftMargin = 0;
    #if defined(OEM_JCI)
    leftMargin = 95;
    #endif
    currentCameraIndex = 1;

    memset(scheduleTimeing, 0, sizeof(scheduleTimeing));
    memset(&copyToCameraFields, 0, sizeof(copyToCameraFields));

    cameraNameSpinbox = new DropDown(SCALE_WIDTH(15),
                                     SCALE_HEIGHT(5),
                                     BGTILE_LARGE_SIZE_WIDTH,
                                     BGTILE_HEIGHT,
                                     REC_CAMERANAME_SPINBOX_CTRL,
                                     DROPDOWNBOX_SIZE_320,
                                     recordingStrings[REC_CAMERANAME_SPINBOX_STRING],
                                     cameraList,
                                     this,
                                     "",
                                     false,
                                     SCALE_WIDTH(20));

    m_elementList[REC_CAMERANAME_SPINBOX_CTRL] = cameraNameSpinbox;

    connect (cameraNameSpinbox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (cameraNameSpinbox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotSpinboxValueChanged(QString,quint32)));

    copyToCameraBtn = new PageOpenButton((cameraNameSpinbox->x () +
                                         cameraNameSpinbox->width ()) - SCALE_WIDTH(210),
                                         cameraNameSpinbox->y () + SCALE_HEIGHT(5),
                                         SCALE_WIDTH(100),
                                         SCALE_HEIGHT(30),
                                         REC_CPY_CAMERA_BTN_CTRL+MAX_WEEKDAYS-1,
                                         PAGEOPENBUTTON_EXTRALARGE,
                                         recordingStrings[REC_CPY_CAMERA_BTN_STRING],
                                         this,
                                         "","",
                                         false,
                                         0,
                                         NO_LAYER);

    m_elementList[REC_CPY_CAMERA_BTN_CTRL+MAX_WEEKDAYS-1] = copyToCameraBtn;

    connect (copyToCameraBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotPageOpenBtnClick(int)));

    connect (copyToCameraBtn,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));


    alarmHeading = new ElementHeading(cameraNameSpinbox->x (),
                                      cameraNameSpinbox->y () +
                                      cameraNameSpinbox->height () + SCALE_HEIGHT(5),
                                      BGTILE_MEDIUM_SIZE_WIDTH,
                                      BGTILE_HEIGHT,
                                      recordingStrings[REC_ALARM_REC_STRING],
                                      UP_LAYER,
                                      this,
                                      false,
                                      SCALE_FONT(20), NORMAL_FONT_SIZE, true);

    cosecHeading = new ElementHeading(alarmHeading->x () +alarmHeading->width () + SCALE_WIDTH(7),
                                      cameraNameSpinbox->y () +
                                      cameraNameSpinbox->height () + SCALE_HEIGHT(5),
                                      BGTILE_MEDIUM_SIZE_WIDTH,
                                      BGTILE_HEIGHT,
                                      recordingStrings[REC_COSEC_HEADING_STRING],
                                      UP_LAYER,
                                      this,
                                      false,
                                      SCALE_FONT(20), NORMAL_FONT_SIZE, true);

    preRecordParam = new TextboxParam();

    preRecordParam->isNumEntry = true;
    preRecordParam->maxChar = 2;
    preRecordParam->isCentre = true;
    preRecordParam->minNumValue = 0;
    preRecordParam->maxNumValue = 30;
    preRecordParam->validation=QRegExp(QString("[0-9]"));
    preRecordParam->labelStr = recordingStrings[REC_PRERECORD_LABEL_STRING];
    preRecordParam->suffixStr = QString("(0 - 30 ") + Multilang("sec") + QString(")");

    preRecordTextBox = new TextBox(cameraNameSpinbox->x (),
                                   alarmHeading->y () +
                                   alarmHeading->height (),
                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                   BGTILE_HEIGHT,
                                   REC_PRERECORD_TEXTBOX_CTRL,
                                   TEXTBOX_EXTRASMALL,
                                   this,
                                   preRecordParam,
                                   MIDDLE_TABLE_LAYER,
                                   true,
                                   false,
                                   false,
                                   leftMargin);

    m_elementList[REC_PRERECORD_TEXTBOX_CTRL] = preRecordTextBox;

    connect (preRecordTextBox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotLoadInfopage(int,INFO_MSG_TYPE_e)));

    connect (preRecordTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    postRecordParam = new TextboxParam();
    postRecordParam->isNumEntry = true;
    postRecordParam->maxChar = 3;
    postRecordParam->isCentre = true;
    postRecordParam->minNumValue = 10;
    postRecordParam->maxNumValue = 300;
    postRecordParam->validation=QRegExp(QString("[0-9]"));
    postRecordParam->labelStr = recordingStrings[REC_POSTRECORD_LABEL_STRING];
    postRecordParam->suffixStr = QString("(10 - 300 ") + Multilang("sec") + QString(")");

    postRecordTextBox = new TextBox(cameraNameSpinbox->x (),
                                    preRecordTextBox->y () +
                                    preRecordTextBox->height (),
                                    BGTILE_MEDIUM_SIZE_WIDTH,
                                    BGTILE_HEIGHT,
                                    REC_POSTRECORD_TEXTBOX_CTRL,
                                    TEXTBOX_EXTRASMALL,
                                    this,
                                    postRecordParam,
                                    BOTTOM_TABLE_LAYER,
                                    true,
                                    false,
                                    false,
                                    leftMargin);

    m_elementList[REC_POSTRECORD_TEXTBOX_CTRL] = postRecordTextBox;

    connect (postRecordTextBox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotLoadInfopage(int,INFO_MSG_TYPE_e)));

    connect (postRecordTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    enablePerRecordCheckBox = new OptionSelectButton(cosecHeading->x (),
                                                     cosecHeading->y () +
                                                     cosecHeading->height (),
                                                     BGTILE_MEDIUM_SIZE_WIDTH,
                                                     BGTILE_HEIGHT,
                                                     CHECK_BUTTON_INDEX,
                                                     recordingStrings[REC_ENBL_PRE_REC_STRING],
                                                     this,
                                                     MIDDLE_TABLE_LAYER,
                                                     -1,
                                                     MX_OPTION_TEXT_TYPE_LABEL,
                                                     NORMAL_FONT_SIZE,
                                                     REC_ENB_PER_REC_CHCKBOX_CTRL);

    connect (enablePerRecordCheckBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (enablePerRecordCheckBox,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotCheckboxClicked(OPTION_STATE_TYPE_e,int)));

    m_elementList[REC_ENB_PER_REC_CHCKBOX_CTRL] = enablePerRecordCheckBox;

    cosecPreRecordParam = new TextboxParam();
    cosecPreRecordParam->isNumEntry = true;
    cosecPreRecordParam->maxChar = 2;
    cosecPreRecordParam->isCentre = true;
    cosecPreRecordParam->minNumValue = 0;
    cosecPreRecordParam->maxNumValue = 30;
    cosecPreRecordParam->validation = QRegExp(QString("[0-9]"));
    cosecPreRecordParam->labelStr = recordingStrings[REC_PRERECORD_LABEL_STRING];
    cosecPreRecordParam->suffixStr = QString("(0 - 30 ") + Multilang("sec") + QString(")");

    cosecPreRecordTextBox = new TextBox(cosecHeading->x (),
                                        enablePerRecordCheckBox->y () +
                                        enablePerRecordCheckBox->height (),
                                        BGTILE_MEDIUM_SIZE_WIDTH,
                                        BGTILE_HEIGHT,
                                        REC_COSEC_PRERECORD_TEXTBOX_CTRL,
                                        TEXTBOX_EXTRASMALL,
                                        this,
                                        cosecPreRecordParam,
                                        BOTTOM_TABLE_LAYER);

    m_elementList[REC_COSEC_PRERECORD_TEXTBOX_CTRL] = cosecPreRecordTextBox;

    connect (cosecPreRecordTextBox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotLoadInfopage(int,INFO_MSG_TYPE_e)));

    connect (cosecPreRecordTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    manualRecording = new OptionSelectButton(postRecordTextBox->x (),
                                             postRecordTextBox->y () +
                                             postRecordTextBox->height () + SCALE_HEIGHT(5),
                                             BGTILE_LARGE_SIZE_WIDTH,
                                             BGTILE_HEIGHT,
                                             CHECK_BUTTON_INDEX,
                                             recordingStrings[REC_MAN_REC_STRING],
                                             this,
                                             COMMON_LAYER,
                                             SCALE_WIDTH(10),
                                             MX_OPTION_TEXT_TYPE_SUFFIX,
                                             NORMAL_FONT_SIZE,
                                             REC_MANUAL_RECORDING_CTRL, true, NORMAL_FONT_COLOR, true);

    m_elementList[REC_MANUAL_RECORDING_CTRL] = manualRecording;

    connect (manualRecording,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotCheckboxClicked(OPTION_STATE_TYPE_e,int)));
    connect (manualRecording,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    scheduleRecording = new OptionSelectButton(manualRecording->x (),
                                               manualRecording->y () +
                                               manualRecording->height () + SCALE_HEIGHT(5),
                                               BGTILE_LARGE_SIZE_WIDTH,
                                               BGTILE_HEIGHT,
                                               CHECK_BUTTON_INDEX,
                                               recordingStrings[REC_SCHD_REC_STRING],
                                               this,
                                               COMMON_LAYER,
                                               SCALE_WIDTH(10),
                                               MX_OPTION_TEXT_TYPE_SUFFIX,
                                               NORMAL_FONT_SIZE,
                                               REC_SCHD_RECORDING_CTRL, true, NORMAL_FONT_COLOR, true);

    m_elementList[REC_SCHD_RECORDING_CTRL] = scheduleRecording;

    connect (scheduleRecording,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotCheckboxClicked(OPTION_STATE_TYPE_e,int)));
    connect (scheduleRecording,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    scheduleWeekly[0] = new ScheduleBar(scheduleRecording->x () ,
                                        scheduleRecording->y () +
                                        scheduleRecording->height () - 1 ,
                                        BGTILE_LARGE_SIZE_WIDTH,
                                        BGTILE_HEIGHT,
                                        MIDDLE_TABLE_LAYER,
                                        "",
                                        0,this,true,true);

    for( quint8 index = 1 ; index < MAX_SCHDBAR ;index++)
    {
        scheduleWeekly[index] = new ScheduleBar(scheduleRecording->x (),
                                                scheduleWeekly[0]->y () +
                                                scheduleWeekly[0]->height () +
                                                BGTILE_HEIGHT*(index-1) ,
                                                BGTILE_LARGE_SIZE_WIDTH,
                                                BGTILE_HEIGHT,
                                                (index < (MAX_SCHDBAR - 1)) ? MIDDLE_TABLE_LAYER : BOTTOM_TABLE_LAYER,
                                                recordingStrings[(index-1) + REC_SCHD_WEEK_STRING],
                                                0, this, false);

        scheduleWeeklySet[index-1] = new PageOpenButton(scheduleRecording->x () +
                                                        scheduleRecording->width () - SCALE_WIDTH(83),
                                                        scheduleWeekly[0]->y () +
                                                        scheduleWeekly[0]->height () +
                                                        BGTILE_HEIGHT*(index-1) ,
                                                        BGTILE_LARGE_SIZE_WIDTH,
                                                        BGTILE_HEIGHT,
                                                        (index -1) +  REC_SCHD_WEEKLY_SET,
                                                        PAGEOPENBUTTON_SMALL,
                                                        recordingStrings[REC_SET_STRING],
                                                        this,"","",
                                                        false,0,NO_LAYER,false);

        m_elementList[(index -1) +  REC_SCHD_WEEKLY_SET] = scheduleWeeklySet[index-1];

        connect (scheduleWeeklySet[index-1] ,
                 SIGNAL(sigButtonClick(int)),
                 this,
                 SLOT(slotPageOpenBtnClick(int)));
        connect (scheduleWeeklySet[index-1],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));

        isScheduleSetForEntireDay[index - 1] = 0;

    }

    cnfgBtnPair[0]->resetGeometry((SCALE_WIDTH(PAGE_RIGHT_PANEL_WIDTH / 2) - SCALE_WIDTH(145)),
                                 (scheduleWeekly[(MAX_SCHDBAR - 1)]->y() + scheduleWeekly[(MAX_SCHDBAR - 1)]->height()) +
                                 (SCALE_HEIGHT(PAGE_RIGHT_PANEL_HEIGHT) - (scheduleWeekly[(MAX_SCHDBAR - 1)]->y() +
                                 scheduleWeekly[(MAX_SCHDBAR - 1)]->height())) / 2);
    cnfgBtnPair[1]->resetGeometry(SCALE_WIDTH(PAGE_RIGHT_PANEL_WIDTH / 2),
                                 (scheduleWeekly[(MAX_SCHDBAR - 1)]->y() + scheduleWeekly[(MAX_SCHDBAR - 1)]->height()) +
                                 (SCALE_HEIGHT(PAGE_RIGHT_PANEL_HEIGHT) - (scheduleWeekly[(MAX_SCHDBAR - 1)]->y() +
                                 scheduleWeekly[(MAX_SCHDBAR - 1)]->height())) / 2);
    cnfgBtnPair[2]->resetGeometry((SCALE_WIDTH(PAGE_RIGHT_PANEL_WIDTH / 2) + SCALE_WIDTH(145)),
                                 (scheduleWeekly[(MAX_SCHDBAR - 1)]->y() + scheduleWeekly[(MAX_SCHDBAR - 1)]->height()) +
                                 (SCALE_HEIGHT(PAGE_RIGHT_PANEL_HEIGHT) - (scheduleWeekly[(MAX_SCHDBAR - 1)]->y() +
                                 scheduleWeekly[(MAX_SCHDBAR - 1)]->height())) / 2);

    #if defined(OEM_JCI)
    cosecHeading->setVisible(false);
    enablePerRecordCheckBox->setVisible(false);
    enablePerRecordCheckBox->setIsEnabled(false);
    cosecPreRecordTextBox->setVisible(false);
    cosecPreRecordTextBox->setIsEnabled(false);

    alarmHeading->resetGeometry(BGTILE_LARGE_SIZE_WIDTH, BGTILE_HEIGHT);
    preRecordTextBox->resetGeometry(BGTILE_LARGE_SIZE_WIDTH, BGTILE_HEIGHT);
    postRecordTextBox->resetGeometry(BGTILE_LARGE_SIZE_WIDTH, BGTILE_HEIGHT);
    #endif
}

Recording::~Recording()
{
    for(quint8 index = 0 ; index < MAX_SCHDBAR ;index++)
    {
        delete scheduleWeekly[index];
        if(index < MAX_WEEKDAYS)
        {
            disconnect (scheduleWeeklySet[index] ,
                        SIGNAL(sigButtonClick(int)),
                        this,
                        SLOT(slotPageOpenBtnClick(int)));

            disconnect (scheduleWeeklySet[index],
                        SIGNAL(sigUpdateCurrentElement(int)),
                        this,
                        SLOT(slotUpdateCurrentElement(int)));

            delete scheduleWeeklySet[index];
        }
    }

    disconnect (scheduleRecording,
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotCheckboxClicked(OPTION_STATE_TYPE_e,int)));

    disconnect (scheduleRecording,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

    delete scheduleRecording;

    disconnect (manualRecording,
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotCheckboxClicked(OPTION_STATE_TYPE_e,int)));

    disconnect (manualRecording,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete manualRecording;

    disconnect (cosecPreRecordTextBox,
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotLoadInfopage(int,INFO_MSG_TYPE_e)));

    disconnect (cosecPreRecordTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete cosecPreRecordTextBox;
    delete cosecPreRecordParam;

    disconnect (enablePerRecordCheckBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (enablePerRecordCheckBox,
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotCheckboxClicked(OPTION_STATE_TYPE_e,int)));
    delete enablePerRecordCheckBox;

    disconnect (postRecordTextBox,
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotLoadInfopage(int,INFO_MSG_TYPE_e)));

    disconnect (postRecordTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete postRecordTextBox;
    delete postRecordParam;

    disconnect (preRecordTextBox,
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotLoadInfopage(int,INFO_MSG_TYPE_e)));

    disconnect (preRecordTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete preRecordTextBox;
    delete preRecordParam;

    delete cosecHeading;
    delete alarmHeading;

    disconnect (copyToCameraBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotPageOpenBtnClick(int)));
    disconnect (copyToCameraBtn,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete copyToCameraBtn;

    disconnect (cameraNameSpinbox,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotSpinboxValueChanged(QString,quint32)));
    disconnect (cameraNameSpinbox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete cameraNameSpinbox;

    if(copytoCamera != NULL)
    {
        disconnect (copytoCamera,
                    SIGNAL(sigDeleteObject(quint8)),
                    this,
                    SLOT(slotSubObjectDelete(quint8)));

        delete copytoCamera;
        copytoCamera = NULL;
    }

    if(setSchedule != NULL)
    {
        disconnect (setSchedule,
                    SIGNAL(sigDeleteObject()),
                    this,
                    SLOT(slotSubObjectDelete()));
        delete setSchedule;
        setSchedule = NULL;
    }
}

void Recording:: getConfig()
{
    fillCameraList();
    memset(&copyToCameraFields, 0, sizeof(copyToCameraFields));
    createPayload(MSG_GET_CFG);
}

void Recording:: defaultConfig()
{
    createPayload(MSG_DEF_CFG);
}

void Recording::createPayload(REQ_MSG_ID_e msgType )
{
    QString payloadString = payloadLib->createDevCnfgPayload(msgType,
                                                             ENABLE_SCH_REC_TABLE_INDEX,
                                                             currentCameraIndex,
                                                             currentCameraIndex,
                                                             RECORDING_FROM_FIELD,
                                                             MAX_REC_ENB_SCH_FIELD_NO,
                                                             MAX_REC_ENB_SCH_FIELD_NO);

    payloadString =  payloadLib->createDevCnfgPayload(msgType,
                                                      SCH_REC_TABLE_INDEX,
                                                      (MAX_WEEKDAYS*(currentCameraIndex - 1) + 1),
                                                      MAX_WEEKDAYS*currentCameraIndex,
                                                      RECORDING_FROM_FIELD,
                                                      MAX_REC_SCH_REC_FIELD_NO,
                                                      MAX_REC_SCH_TOTAL_FEILD,
                                                      payloadString,
                                                      MAX_REC_ENB_SCH_FIELD_NO);

    payloadString = payloadLib->createDevCnfgPayload (msgType,
                                                      ALARM_REC_TABLE_INDEX,
                                                      currentCameraIndex,
                                                      currentCameraIndex,
                                                      RECORDING_FROM_FIELD,
                                                      MAX_REC_ALM_REC_FIELD_NO,
                                                      MAX_REC_ALM_REC_FIELD_NO,
                                                      payloadString,
                                                      REC_ALARM_START_FEILD);

    payloadString = payloadLib->createDevCnfgPayload (msgType,
                                                      COSEC_RECORDING_TABLE_INDEX,
                                                      currentCameraIndex,
                                                      currentCameraIndex,
                                                      RECORDING_FROM_FIELD,
                                                      MAX_REC_COS_REC_FIELD_NO,
                                                      MAX_REC_COS_REC_FIELD_NO,
                                                      payloadString,
                                                      REC_COSEC_START_FEILD);

    payloadString = payloadLib->createDevCnfgPayload (msgType,
                                                      MANUAL_RECORDING_TABLE_INDEX,
                                                      currentCameraIndex,
                                                      currentCameraIndex,
                                                      RECORDING_FROM_FIELD,
                                                      MAX_REC_MANUAL_REC_FIELD_NO,
                                                      MAX_REC_MANUAL_REC_FIELD_NO,
                                                      payloadString,
                                                      REC_MANUAL_REC_START_FEILD);

    DevCommParam* param = new DevCommParam();
    param->msgType = msgType;
    param->payload = payloadString;

    processBar->loadProcessBar ();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

/* This function passes the Values of EntireDay and adaptive flags as well as time slots of each day to server for each day */
void Recording::fillRecords()
{
    QString stopTime = "";
    QString startTime = "";

    for(quint8 index = 0;index < MAX_WEEKDAYS; index++)
    {
        /* If entire day is selected no need to pass the value of time to the server as it would be handled to 0-24 by GUI itself */
        payloadLib->setCnfgArrayAtIndex (REC_SCH_ENTIRE_DAY +
                                         ((index)*MAX_REC_SCH_REC_FIELD_NO),isScheduleSetForEntireDay[index]);

        /* If entire day flag is not true then the time slots must be passed to server only if that day is checked in copy to day parametr of setschedule  */
        if(copyToWeekdaysFields)
        {
            for(quint8 index1 = 0 ; index1 < MAX_TIME_SLOTS ;index1++)
            {
                if(scheduleTimeing[index].start_hour[index1] > 24)
                    scheduleTimeing[index].start_hour[index1] = 0;

                scheduleTimeing[index].start_hour[index1] < 10 ?
                            startTime = ( QString("0") + QString("%1").arg (scheduleTimeing[index].start_hour[index1])) :
                        startTime = ( QString("%1").arg (scheduleTimeing[index].start_hour[index1]) );

                if(scheduleTimeing[index].start_min[index1] > 60)
                    scheduleTimeing[index].start_min[index1] = 0;

                scheduleTimeing[index].start_min[index1] < 10 ?
                            startTime.append( QString("0") + QString("%1").arg(scheduleTimeing[index].start_min[index1]) ) :
                            startTime.append (QString("%1").arg(scheduleTimeing[index].start_min[index1]));

                payloadLib->setCnfgArrayAtIndex((REC_SCH_START_TIME1
                                                 + (MAX_REC_SCH_REC_FIELD_NO * (index) )
                                                 + index1 + 1*(index1)),startTime);

                if(scheduleTimeing[index].stop_hour[index1] > 24)
                    scheduleTimeing[index].stop_hour[index1] = 0;

                scheduleTimeing[index].stop_hour[index1] < 10 ?
                            stopTime =(QString("0") + QString("%1").arg (scheduleTimeing[index].stop_hour[index1]) ):
                        stopTime =( QString("%1").arg (scheduleTimeing[index].stop_hour[index1]) );

                if(scheduleTimeing[index].stop_min[index1] > 60)
                    scheduleTimeing[index].stop_min[index1] = 0;

                scheduleTimeing[index].stop_min[index1] < 10 ?
                            stopTime.append (QString("0") + QString("%1").arg(scheduleTimeing[index].stop_min[index1]) ):
                            stopTime.append (QString("%1").arg(scheduleTimeing[index].stop_min[index1]));

                payloadLib->setCnfgArrayAtIndex((REC_SCH_STOP_TIME1
                                                 + (MAX_REC_SCH_REC_FIELD_NO * (index))
                                                 + index1 +  1*(index1)),stopTime);

            }
        }
    }
}

void Recording::fillRecordsForAlarmSchedule()
{
    SET_CAMERA_MASK_BIT(copyToCameraFields, (currentCameraIndex - 1));

    for (quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
    {
        payloadLib->setCnfgArrayAtIndex(REC_CPY_TO_CAMERA_START + maskIdx, copyToCameraFields.bitMask[maskIdx]);
        payloadLib->setCnfgArrayAtIndex(REC_ALM_CPY_TO_CAMERA_START + maskIdx, copyToCameraFields.bitMask[maskIdx]);
        payloadLib->setCnfgArrayAtIndex(REC_COSEC_CPY_TO_CAMERA_START + maskIdx, copyToCameraFields.bitMask[maskIdx]);
        payloadLib->setCnfgArrayAtIndex(REC_MANUAL_CPY_TO_CAMERA_START + maskIdx, copyToCameraFields.bitMask[maskIdx]);
    }

    for(quint8 index = 0 ; index < MAX_WEEKDAYS ; index++)
    {
        for (quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
        {
            payloadLib->setCnfgArrayAtIndex((REC_SCH_CPY_TO_CAMERA_START + maskIdx) + (index)*MAX_REC_SCH_REC_FIELD_NO,
                                            copyToCameraFields.bitMask[maskIdx]);
        }
    }

    payloadLib->setCnfgArrayAtIndex(REC_ENB_SCHD_REC, (scheduleRecording->getCurrentState() == OFF_STATE ? 0 : 1));
    payloadLib->setCnfgArrayAtIndex(REC_MANUAL_ENB_REC, manualRecording->getCurrentState() == OFF_STATE ? 0 : 1);
    payloadLib->setCnfgArrayAtIndex(REC_COSEC_ENB_REC, enablePerRecordCheckBox->getCurrentState() == OFF_STATE ? 0 : 1);
    payloadLib->setCnfgArrayAtIndex(REC_COSEC_PRE_RECORD_VALUE, cosecPreRecordTextBox->getInputText ());
    payloadLib->setCnfgArrayAtIndex(REC_PRE_RECORD_TIME, preRecordTextBox->getInputText ());
    payloadLib->setCnfgArrayAtIndex(REC_POST_RECORD_TIME, postRecordTextBox->getInputText ());
}

void Recording::saveConfig()
{
    if (IS_VALID_OBJ(postRecordTextBox))
    {
        if (!(postRecordTextBox->doneKeyValidation()))
        {
            return;
        }
    }

    fillRecordsForAlarmSchedule();
    createPayload (MSG_SET_CFG);
}

void Recording::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    bool isUnloadProcessbar = true;

    if(deviceName == currDevName)
    {
        switch(param->deviceStatus)
        {
        case CMD_SUCCESS:
        {
            switch(param->msgType)
            {
            case MSG_GET_CFG:
            {
                payloadLib->parsePayload (param->msgType, param->payload);
                if((payloadLib->getcnfgTableIndex(0) == ENABLE_SCH_REC_TABLE_INDEX)
                        && (payloadLib->getcnfgTableIndex(1) == SCH_REC_TABLE_INDEX)
                        && (payloadLib->getcnfgTableIndex(2) == ALARM_REC_TABLE_INDEX)
                        && (payloadLib->getcnfgTableIndex(3) == COSEC_RECORDING_TABLE_INDEX)
                        && (payloadLib->getcnfgTableIndex(4) == MANUAL_RECORDING_TABLE_INDEX))
                {
                    m_configResponse.clear();

                    for (quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
                    {
                        m_configResponse[REC_CPY_TO_CAMERA_START + maskIdx] =
                                payloadLib->getCnfgArrayAtIndex(REC_CPY_TO_CAMERA_START + maskIdx).toUInt();
                        m_configResponse[REC_SCH_CPY_TO_CAMERA_START + maskIdx] =
                                payloadLib->getCnfgArrayAtIndex(REC_SCH_CPY_TO_CAMERA_START + maskIdx).toUInt();
                        m_configResponse[REC_ALM_CPY_TO_CAMERA_START + maskIdx] =
                                payloadLib->getCnfgArrayAtIndex(REC_ALM_CPY_TO_CAMERA_START + maskIdx).toUInt();
                        m_configResponse[REC_COSEC_CPY_TO_CAMERA_START + maskIdx] =
                                payloadLib->getCnfgArrayAtIndex(REC_COSEC_CPY_TO_CAMERA_START + maskIdx).toUInt();
                        m_configResponse[REC_MANUAL_CPY_TO_CAMERA_START + maskIdx] =
                                payloadLib->getCnfgArrayAtIndex(REC_MANUAL_CPY_TO_CAMERA_START + maskIdx).toUInt();
                    }

                    quint16 isEnable = payloadLib->getCnfgArrayAtIndex(REC_ENB_SCHD_REC).toUInt();
                    m_configResponse[REC_ENB_SCHD_REC] = isEnable;
                    scheduleRecording->changeState (isEnable == 0 ? OFF_STATE : ON_STATE);

                    if(isEnable)
                    {
                        for(quint8 index = 1 ; index < MAX_SCHDBAR; index++)
                        {
                            scheduleWeekly[index]->setIsEnable (true);
                            scheduleWeeklySet[index - 1]->setIsEnabled (true);
                        }
                    }
                    else
                    {
                        for(quint8 index = 1 ; index < MAX_SCHDBAR; index++)
                        {
                            scheduleWeekly[index]->setIsEnable (false);
                            scheduleWeeklySet[index - 1]->setIsEnabled (false);
                        }
                    }

                    memset(scheduleTimeing,0,sizeof(scheduleTimeing));

                    for(quint8 index = 0 ; index < MAX_WEEKDAYS; index++)
                    {
                        isScheduleSetForEntireDay[index] = payloadLib->getCnfgArrayAtIndex(REC_SCH_ENTIRE_DAY + (index * MAX_REC_SCH_REC_FIELD_NO)).toUInt ();
                        m_configResponse[(REC_SCH_ENTIRE_DAY + (index * MAX_REC_SCH_REC_FIELD_NO))] = isScheduleSetForEntireDay[index];
                        scheduleWeekly[index + 1]->setScheduleForEntireDay ((isScheduleSetForEntireDay[index] & IS_SET_SCHD_ENTIREDAY_CHECKED) ? true : false);

                        for(quint8 index1 = 0 ; index1 < MAX_TIME_SLOTS ;index1++)
                        {
                            QString time;
                            quint8 temp = REC_SCH_START_TIME1 + (MAX_REC_SCH_REC_FIELD_NO * (index)) + index1 + 1*(index1);
                            m_configResponse[temp] = payloadLib->getCnfgArrayAtIndex(temp);

                            time = payloadLib->getCnfgArrayAtIndex(temp).toString().mid(0,2);
                            scheduleTimeing[index].start_hour[index1] = time.toUInt();

                            time = payloadLib->getCnfgArrayAtIndex(temp).toString().mid(2,2);
                            scheduleTimeing[index].start_min[index1] = time.toUInt();

                            temp = REC_SCH_STOP_TIME1 + (MAX_REC_SCH_REC_FIELD_NO * (index)) + index1 + 1*(index1);
                            m_configResponse[temp] = payloadLib->getCnfgArrayAtIndex(temp);

                            time = payloadLib->getCnfgArrayAtIndex(temp).toString().mid(0,2);
                            scheduleTimeing[index].stop_hour[index1] = time.toUInt();

                            time = payloadLib->getCnfgArrayAtIndex(temp).toString().mid(2,2);
                            scheduleTimeing[index].stop_min[index1] = time.toUInt();
                        }

                        if(false == (isScheduleSetForEntireDay[index] & IS_SET_SCHD_ENTIREDAY_CHECKED))
                        {                            
                            scheduleWeekly[index+1]->setSchedule (scheduleTimeing[index]);
                        }
                    }

                    preRecordTextBox->setInputText(payloadLib->getCnfgArrayAtIndex(REC_PRE_RECORD_TIME).toString());
                    m_configResponse[REC_PRE_RECORD_TIME] = preRecordTextBox->getInputText();

                    postRecordTextBox->setInputText(payloadLib->getCnfgArrayAtIndex(REC_POST_RECORD_TIME).toString());
                    m_configResponse[REC_POST_RECORD_TIME] = postRecordTextBox->getInputText();

                    enablePerRecordCheckBox->changeState((payloadLib->getCnfgArrayAtIndex(REC_COSEC_ENB_REC).toUInt() == 0) ? OFF_STATE : ON_STATE);
                    m_configResponse[REC_COSEC_ENB_REC] = payloadLib->getCnfgArrayAtIndex(REC_COSEC_ENB_REC).toUInt();

                    cosecPreRecordTextBox->setIsEnabled((payloadLib->getCnfgArrayAtIndex(REC_COSEC_ENB_REC).toUInt() == 0) ? false : true);
                    cosecPreRecordTextBox->setInputText(payloadLib->getCnfgArrayAtIndex(REC_COSEC_PRE_RECORD_VALUE).toString());
                    m_configResponse[REC_COSEC_PRE_RECORD_VALUE] = cosecPreRecordTextBox->getInputText();

                    isEnable = payloadLib->getCnfgArrayAtIndex(REC_MANUAL_ENB_REC).toUInt();
                    m_configResponse[REC_MANUAL_ENB_REC] = isEnable;

                    manualRecording->changeState((isEnable == 0) ? OFF_STATE : ON_STATE);
                }
            }
                break;

            case MSG_SET_CFG:
            {
                isUnloadProcessbar = false;
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
                currentCameraIndex = cameraNameSpinbox->getIndexofCurrElement () + 1;
                getConfig ();
            }
                break;

            case MSG_DEF_CFG:
            {
                isUnloadProcessbar = false;
                currentCameraIndex = cameraNameSpinbox->getIndexofCurrElement () + 1;
                getConfig ();
            }
                break;

            default:
                break;
            }//inner switch

        }
            break;

        default:
            processBar->unloadProcessBar ();
            infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
            break;
        }
    }

    if(isUnloadProcessbar)
    {
        processBar->unloadProcessBar ();
    }
    update ();
}

void Recording::slotLoadInfopage (int index, INFO_MSG_TYPE_e msgtype)
{
    if(msgtype == INFO_MSG_ERROR)
    {
        infoPage->loadInfoPage(Multilang("Please enter") + QString(" ") +
                               QString((index == REC_COSEC_PRERECORD_TEXTBOX_CTRL) ? ("COSEC") : ("Alarm")) + QString(" ") +
                               Multilang(QString(recordingStrings[(index == REC_POSTRECORD_TEXTBOX_CTRL ? REC_POSTRECORD_LABEL_STRING : REC_PRERECORD_LABEL_STRING)]).toUtf8().constData()) +
                               QString(" ") + Multilang("duration in defined range"));
    }
}

void Recording::slotCheckboxClicked(OPTION_STATE_TYPE_e status,int index)
{
    switch(index)
    {
    case REC_SCHD_RECORDING_CTRL:
    {
        if(status)
        {
            for(quint8 index = 1 ; index < MAX_SCHDBAR; index++)
            {
                scheduleWeekly[index]->setIsEnable (true);
                scheduleWeeklySet[index - 1]->setIsEnabled (true);
            }
        }
        else
        {
            for(quint8 index = 1 ; index < MAX_SCHDBAR; index++)
            {
                scheduleWeekly[index]->setIsEnable (false);
                scheduleWeeklySet[index - 1]->setIsEnabled (false);
            }
        }
    }
        break;

    case REC_ENB_PER_REC_CHCKBOX_CTRL:
        cosecPreRecordTextBox->setIsEnabled (status == ON_STATE ? true : false);
        break;

    default:
        break;
    }
}

void Recording::slotPageOpenBtnClick (int index)
{
    if(index == REC_CPY_CAMERA_BTN_CTRL + MAX_WEEKDAYS - 1)
    {
        if(copytoCamera != NULL)
        {
            return;
        }

        memset(&copyToCameraFields, 0, sizeof(copyToCameraFields));
        SET_CAMERA_MASK_BIT(copyToCameraFields, (currentCameraIndex - 1));

        for (quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
        {
            m_configResponse[REC_CPY_TO_CAMERA_START + maskIdx] = copyToCameraFields.bitMask[maskIdx];
            m_configResponse[REC_SCH_CPY_TO_CAMERA_START + maskIdx] = copyToCameraFields.bitMask[maskIdx];
            m_configResponse[REC_ALM_CPY_TO_CAMERA_START + maskIdx] = copyToCameraFields.bitMask[maskIdx];
            m_configResponse[REC_COSEC_CPY_TO_CAMERA_START + maskIdx] = copyToCameraFields.bitMask[maskIdx];
            m_configResponse[REC_MANUAL_CPY_TO_CAMERA_START + maskIdx] = copyToCameraFields.bitMask[maskIdx];
        }

        copytoCamera = new CopyToCamera(cameraList,
                                        copyToCameraFields,
                                        parentWidget(),
                                        recordingStrings[REC_COPY_TO_CAM_HEADING],
                                        REC_CPY_CAMERA_BTN_CTRL);
        connect (copytoCamera,
                 SIGNAL(sigDeleteObject(quint8)),
                 this,
                 SLOT(slotSubObjectDelete(quint8)));

    }
    else
    {
        currentDaySelect = (index - REC_SCHD_WEEKLY_SET);
        if(setSchedule != NULL)
        {
            return;
        }

        copyToWeekdaysFields = 0;
        setSchedule = new SetSchedule( &scheduleTimeing[currentDaySelect],
                                       &isScheduleSetForEntireDay[currentDaySelect],
                                       &copyToWeekdaysFields,
                                       parentWidget (),
                                       currentDaySelect);
        connect (setSchedule,
                 SIGNAL(sigDeleteObject()),
                 this,
                 SLOT(slotSubObjectDelete()));
    }
}

void Recording::updateData ()
{
    for(quint8 index = 0; index < MAX_WEEKDAYS; index++)
    {
        /* updates all the setSchedule start and stop time as well as EntireDay and Adaptiveflags value to the days selected in copy days parameter of setschedule */
        if(copyToWeekdaysFields & (1<<index))
        {
            isScheduleSetForEntireDay[index] = isScheduleSetForEntireDay[currentDaySelect];
            scheduleWeekly[index+1]->setScheduleForEntireDay ((isScheduleSetForEntireDay[index] & IS_SET_SCHD_ENTIREDAY_CHECKED) ? true : false);
            if(false == (isScheduleSetForEntireDay[currentDaySelect] & IS_SET_SCHD_ENTIREDAY_CHECKED))
            {
                scheduleWeekly[index+1]->setSchedule(scheduleTimeing[currentDaySelect]);
            }

            /* copy currentDay schedule timing to other day if flag for copyToWeekDay is selected for that day */
            if(index != currentDaySelect)
            {
                scheduleTimeing[index] = scheduleTimeing[currentDaySelect];
            }
        }
    }
}

void Recording::handleInfoPageMessage(int index)
{
    if(index == INFO_OK_BTN)
    {
        if(infoPage->getText() == (ValidationMessage::getValidationMessage(SAVE_CHANGES)))
        {
            saveConfig();
        }
    }
    else
    {
        currentCameraIndex = cameraNameSpinbox->getIndexofCurrElement () + 1;
        getConfig ();
    }
}

bool Recording::isUserChangeConfig()
{
    if(m_configResponse.isEmpty())
    {
        return false;
    }

    if(m_configResponse[REC_ENB_SCHD_REC] != scheduleRecording->getCurrentState())
    {
        return true;
    }

    for (quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
    {
        if(m_configResponse[REC_CPY_TO_CAMERA_START + maskIdx] != copyToCameraFields.bitMask[maskIdx])
        {
            return true;
        }

        if(m_configResponse[REC_SCH_CPY_TO_CAMERA_START + maskIdx] != copyToCameraFields.bitMask[maskIdx])
        {
            return true;
        }

        if(m_configResponse[REC_ALM_CPY_TO_CAMERA_START + maskIdx] != copyToCameraFields.bitMask[maskIdx])
        {
            return true;
        }

        if(m_configResponse[REC_COSEC_CPY_TO_CAMERA_START + maskIdx] != copyToCameraFields.bitMask[maskIdx])
        {
            return true;
        }

        if(m_configResponse[REC_MANUAL_CPY_TO_CAMERA_START + maskIdx] != copyToCameraFields.bitMask[maskIdx])
        {
            return true;
        }
    }

    for(quint8 index = 0; index < MAX_WEEKDAYS; index++)
    {
        if(m_configResponse[(REC_SCH_ENTIRE_DAY + (index * MAX_REC_SCH_REC_FIELD_NO))] != isScheduleSetForEntireDay[index])
        {
            return true;
        }

        for(quint8 index1 = 0; index1 < MAX_TIME_SLOTS; index1++)
        {
            quint8 temp = REC_SCH_START_TIME1 + (MAX_REC_SCH_REC_FIELD_NO * (index)) + index1 +  1*(index1);

            if(m_configResponse[temp].toString().mid(0,2).toUInt() != scheduleTimeing[index].start_hour[index1])
            {
                return true;
            }

            if( m_configResponse[temp].toString().mid(2,2).toUInt() != scheduleTimeing[index].start_min[index1])
            {
                return true;
            }

            temp = REC_SCH_STOP_TIME1 + (MAX_REC_SCH_REC_FIELD_NO * (index)) + index1 + 1*(index1);

            if( m_configResponse[temp].toString().mid(0,2).toUInt() != scheduleTimeing[index].stop_hour[index1])
            {
                return true;
            }

            if( m_configResponse[temp].toString().mid(2,2).toUInt() != scheduleTimeing[index].stop_min[index1])
            {
                return true;
            }
        }
    }

    if(m_configResponse[REC_PRE_RECORD_TIME] != preRecordTextBox->getInputText())
    {
        return true;
    }

    if(m_configResponse[REC_POST_RECORD_TIME] != postRecordTextBox->getInputText())
    {
        return true;
    }

    if(m_configResponse[REC_COSEC_ENB_REC] != enablePerRecordCheckBox->getCurrentState())
    {
        return true;
    }

    if(m_configResponse[REC_COSEC_PRE_RECORD_VALUE] !=  cosecPreRecordTextBox->getInputText())
    {
        return true;
    }

    if(m_configResponse[REC_MANUAL_ENB_REC] !=  manualRecording->getCurrentState())
    {
        return true;
    }

    return false;
}

void Recording::slotSubObjectDelete()
{
    updateData();

    if(setSchedule != NULL)
    {
        disconnect (setSchedule,
                    SIGNAL(sigDeleteObject()),
                    this,
                    SLOT(slotSubObjectDelete()));
        delete setSchedule;
        setSchedule = NULL;
    }

    fillRecords();

    m_currentElement = (currentDaySelect + REC_SCHD_WEEKLY_SET);
    m_elementList[m_currentElement]->forceActiveFocus ();
}

void Recording::slotSubObjectDelete (quint8 index)
{
    if(index == REC_CPY_CAMERA_BTN_CTRL)
    {
        if(copytoCamera != NULL)
        {
            disconnect (copytoCamera,
                        SIGNAL(sigDeleteObject(quint8)),
                        this,
                        SLOT(slotSubObjectDelete(quint8)));

            delete copytoCamera;
            copytoCamera = NULL;
        }
    }

    fillRecords();

    if(IS_VALID_OBJ(m_elementList[m_currentElement]))
    {
        m_elementList[m_currentElement]->forceActiveFocus ();
    }
}

void Recording::slotSpinboxValueChanged(QString ,quint32)
{
    if(isUserChangeConfig())
    {
        infoPage->loadInfoPage(ValidationMessage::getValidationMessage(SAVE_CHANGES), true, false, "", CONFORMATION_BTN_YES, CONFORMATION_BTN_NO);
    }
    else
    {
        currentCameraIndex = cameraNameSpinbox->getIndexofCurrElement () + 1;
        getConfig ();
    }
}
