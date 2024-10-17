#include "SnapshotSchedule.h"
#include "ValidationMessage.h"

#define SNAPSHT_TILE_WIDTH                 SCALE_WIDTH(680)
#define SNAPSHT_CONFIG_MAX_FIELDS          (SNPSHT_SCHD_FIELDS_COPY_TO_CAM_END + 1)
#define SNAPSHT_SCHD_CONFIG_MAX_FIELDS     (SNPSHT_SCHD_FIELDS_SCHD_COPY_TO_CAM_END - SNPSHT_SCHD_FIELDS_ENTRDAY + 1)
#define MAX_SCHD_TIME_SLOT 6
#define LEFT_MARGIN_FROM_CENTER             SCALE_WIDTH(30)

typedef enum{

    SNPSHT_SCHD_CAMLIST_DROPBOX,
    SNPSHT_SCHD_ENBL_SCHD,
    SNPSHT_SCHD_UPLOAD_DROPBOX,
    SNPSHT_SCHD_EMAIL_TEXTBOX,
    SNPSHT_SCHD_SUBJECT_TEXTBOX,
    SNPSHT_SCHD_MESSAGE_TEXTBOX,
    SNPSHT_SCHD_MAX_IMAGE_RATE_DROPBOX,
    SNPSHT_SCHD_SET_BUTTON,
    SNPSHT_SCHD_COPYTOCAM,
    MAX_SNPSHT_SCHD_CTRL

}SNPSHT_SCHD_CTRL_e;

typedef enum{

    SNPSHT_SCHD_FIELDS_ENB_SCHD,
    SNPSHT_SCHD_FIELDS_UPLD_LOCATION,
    SNPSHT_SCHD_FIELDS_EMAIL_ADDRESS,
    SNPSHT_SCHD_FIELDS_SUBJECT,
    SNPSHT_SCHD_FIELDS_MESSAGE,
    SNPSHT_SCHD_FIELDS_IMAGES_PER_MIN,
    SNPSHT_SCHD_FIELDS_COPY_TO_CAM_START,
    SNPSHT_SCHD_FIELDS_COPY_TO_CAM_END = SNPSHT_SCHD_FIELDS_COPY_TO_CAM_START + CAMERA_MASK_MAX - 1,
    SNPSHT_SCHD_FIELDS_ENTRDAY,
    SNPSHT_SCHD_FIELDS_START_TIME1,
    SNPSHT_SCHD_FIELDS_END_TIME1,
    SNPSHT_SCHD_FIELDS_START_TIME2,
    SNPSHT_SCHD_FIELDS_END_TIME2,
    SNPSHT_SCHD_FIELDS_START_TIME3,
    SNPSHT_SCHD_FIELDS_END_TIME3,
    SNPSHT_SCHD_FIELDS_START_TIME4,
    SNPSHT_SCHD_FIELDS_END_TIME4,
    SNPSHT_SCHD_FIELDS_START_TIME5,
    SNPSHT_SCHD_FIELDS_END_TIME5,
    SNPSHT_SCHD_FIELDS_START_TIME6,
    SNPSHT_SCHD_FIELDS_END_TIME6,
    SNPSHT_SCHD_FIELDS_COPY_WEEKDAYS,
    SNPSHT_SCHD_FIELDS_SCHD_COPY_TO_CAM_START,
    SNPSHT_SCHD_FIELDS_SCHD_COPY_TO_CAM_END = SNPSHT_SCHD_FIELDS_SCHD_COPY_TO_CAM_START + CAMERA_MASK_MAX - 1,
    MAX_SNPSHT_SCHD_FIELDS

}SNPSHT_SCHD_FIELDS_e;


static const QString snapSchdStrings[] =
{
    "Camera",

    "Enable Snapshot schedule",
    "Upload To",
    "Email ID",
    "Subject",
    "Message",
    "Maximum images per minute",
    "Schedule",
    "Copy to Camera"
};

static const QStringList uploadServers = QStringList()  <<  "Email Server"
                                                         <<  "FTP Server 1"
                                                          <<  "FTP Server 2"
                                                           <<  "Network Drive 1"
                                                            <<  "Network Drive 2";

SnapshotSchedule::SnapshotSchedule(QString deviceName,
                                   QWidget* parent,
                                   DEV_TABLE_INFO_t *devTabInfo)
    :ConfigPageControl(deviceName, parent,MAX_SNPSHT_SCHD_CTRL, devTabInfo),
      copyToWeekdaysFields(0), isScheduleSetForEntireDay(0)
{
    memset(&copyToCameraFields, 0, sizeof(copyToCameraFields));
    createDefaultComponents ();
    SnapshotSchedule::getConfig ();
}

SnapshotSchedule::~SnapshotSchedule()
{

    disconnect (cameraListDropDownBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (cameraListDropDownBox,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotDropDownValueChange(QString,quint32)));
    delete cameraListDropDownBox;

    disconnect (copyToCameraBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotPageOpenBtnClick(int)));
    disconnect (copyToCameraBtn,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete copyToCameraBtn;

    disconnect (enableSnapshotCheckBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (enableSnapshotCheckBox,
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotCheckboxClicked(OPTION_STATE_TYPE_e,int)));
    delete enableSnapshotCheckBox;

    disconnect (uploadListDropDownBox,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotDropDownValueChange(QString,quint32)));
    disconnect (uploadListDropDownBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete uploadListDropDownBox;

    disconnect (emailTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (emailTextBox,
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotTextBoxInfoPage(int,INFO_MSG_TYPE_e)));
    delete emailTextBox;
    delete emailTextBoxParam;

    disconnect (subjectTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete subjectTextBox;
    delete subjectTextBoxParam;

    disconnect (msgBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete msgBox;

    disconnect (imageRateDropDownBox,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotDropDownValueChange(QString,quint32)));
    disconnect (imageRateDropDownBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete imageRateDropDownBox;

    disconnect (schdSetBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotPageOpenBtnClick(int)));
    disconnect (schdSetBtn,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete schdSetBtn;

    if(copytoCamera != NULL)
    {
        disconnect (copytoCamera,
                    SIGNAL(sigDeleteObject(quint8)),
                    this,
                    SLOT(slotCopytoCamDelete(quint8)));
        delete copytoCamera;
        copytoCamera = NULL;
    }

    if(setSchedule != NULL)
    {
        disconnect (setSchedule,
                    SIGNAL(sigDeleteObject()),
                    this,
                    SLOT(slotSchdObjectDelete()));
        delete setSchedule;
        setSchedule = NULL;
    }

    if(IS_VALID_OBJ(emailNoteLabel))
    {
        DELETE_OBJ(emailNoteLabel);
    }

}

void SnapshotSchedule::createDefaultComponents()
{
    setSchedule = NULL;
    copytoCamera = NULL;
    emailNoteLabel = NULL;

    currentCameraIndex = 1;
    cameraListDropDownBox = new DropDown((SCALE_WIDTH(PAGE_RIGHT_PANEL_WIDTH) - SNAPSHT_TILE_WIDTH)/2 + SCALE_WIDTH(5),
                                         SCALE_HEIGHT(140),
                                         SNAPSHT_TILE_WIDTH,
                                         BGTILE_HEIGHT,
                                         SNPSHT_SCHD_CAMLIST_DROPBOX,
                                         DROPDOWNBOX_SIZE_320,
                                         snapSchdStrings[SNPSHT_SCHD_CAMLIST_DROPBOX],
                                         cameraList,
                                         this,
                                         "",
                                         false,
                                         SCALE_WIDTH(20));

    m_elementList[SNPSHT_SCHD_CAMLIST_DROPBOX] = cameraListDropDownBox;

    connect (cameraListDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    connect (cameraListDropDownBox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotDropDownValueChange(QString,quint32)));


    copyToCameraBtn = new PageOpenButton((cameraListDropDownBox->x () +
                                          cameraListDropDownBox->width () ) - SCALE_WIDTH(210),
                                         cameraListDropDownBox->y () + SCALE_HEIGHT(5),
                                         SCALE_WIDTH(100),
                                         SCALE_HEIGHT(30),
                                         SNPSHT_SCHD_COPYTOCAM,
                                         PAGEOPENBUTTON_EXTRALARGE,
                                         snapSchdStrings[SNPSHT_SCHD_COPYTOCAM],
                                         this,
                                         "","",
                                         false,
                                         0,
                                         NO_LAYER);

    m_elementList[SNPSHT_SCHD_COPYTOCAM] = copyToCameraBtn;

    connect (copyToCameraBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotPageOpenBtnClick(int)));

    connect (copyToCameraBtn,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));


    enableSnapshotCheckBox = new OptionSelectButton(cameraListDropDownBox->x (),
                                                    cameraListDropDownBox->y () +
                                                    cameraListDropDownBox->height (),
                                                    cameraListDropDownBox->width (),
                                                    cameraListDropDownBox->height (),
                                                    CHECK_BUTTON_INDEX,
                                                    snapSchdStrings[SNPSHT_SCHD_ENBL_SCHD],
                                                    this,
                                                    COMMON_LAYER,
                                                    SCALE_WIDTH(20),
                                                    MX_OPTION_TEXT_TYPE_SUFFIX,
                                                    NORMAL_FONT_SIZE,
                                                    SNPSHT_SCHD_ENBL_SCHD, true, NORMAL_FONT_COLOR, true);

    m_elementList[SNPSHT_SCHD_ENBL_SCHD] = enableSnapshotCheckBox;

    connect (enableSnapshotCheckBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (enableSnapshotCheckBox,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotCheckboxClicked(OPTION_STATE_TYPE_e,int)));


    QMap<quint8, QString> uploadServersList;
    uploadServersList.clear ();

    for(quint8 index = 0; index < uploadServers.length (); index++)
    {
        uploadServersList.insert (index,uploadServers.at(index));
    }

    uploadListDropDownBox = new DropDown(cameraListDropDownBox->x (),
                                         enableSnapshotCheckBox->y () + BGTILE_HEIGHT,
                                         SNAPSHT_TILE_WIDTH,
                                         BGTILE_HEIGHT,
                                         SNPSHT_SCHD_UPLOAD_DROPBOX,
                                         DROPDOWNBOX_SIZE_225,
                                         snapSchdStrings[SNPSHT_SCHD_UPLOAD_DROPBOX],
                                         uploadServersList,
                                         this, "", true, 0, COMMON_LAYER, 
										 true, 8, false, false, 5, 
										 LEFT_MARGIN_FROM_CENTER);

    m_elementList[SNPSHT_SCHD_UPLOAD_DROPBOX] = uploadListDropDownBox;

    connect (uploadListDropDownBox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotDropDownValueChange(QString,quint32)));

    connect (uploadListDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));


    emailTextBoxParam = new TextboxParam();
    emailTextBoxParam->maxChar = 100;
    emailTextBoxParam->isEmailAddrType = true;
    emailTextBoxParam->labelStr = snapSchdStrings[SNPSHT_SCHD_EMAIL_TEXTBOX];

    emailTextBox = new TextBox(cameraListDropDownBox->x (),
                               uploadListDropDownBox->y () + BGTILE_HEIGHT,
                               SNAPSHT_TILE_WIDTH,
                               BGTILE_HEIGHT,
                               SNPSHT_SCHD_EMAIL_TEXTBOX,
                               TEXTBOX_ULTRALARGE,
                               this,
                               emailTextBoxParam,
                               COMMON_LAYER,
                               false, false, false,
                               LEFT_MARGIN_FROM_CENTER);

    m_elementList[SNPSHT_SCHD_EMAIL_TEXTBOX] = emailTextBox;

    connect (emailTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (emailTextBox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotTextBoxInfoPage(int,INFO_MSG_TYPE_e)));

    subjectTextBoxParam = new TextboxParam();
    subjectTextBoxParam->maxChar = 50;
    subjectTextBoxParam->labelStr = snapSchdStrings[SNPSHT_SCHD_SUBJECT_TEXTBOX];

    subjectTextBox = new TextBox(cameraListDropDownBox->x (),
                                 emailTextBox->y () + BGTILE_HEIGHT,
                                 SNAPSHT_TILE_WIDTH,
                                 BGTILE_HEIGHT,
                                 SNPSHT_SCHD_SUBJECT_TEXTBOX,
                                 TEXTBOX_ULTRALARGE,
                                 this,
                                 subjectTextBoxParam,
                                 COMMON_LAYER,false, false, false,
                                 LEFT_MARGIN_FROM_CENTER);

    m_elementList[SNPSHT_SCHD_SUBJECT_TEXTBOX] = subjectTextBox;

    connect (subjectTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));


    msgBox =  new MessageBox(cameraListDropDownBox->x (),
                             subjectTextBox->y () + BGTILE_HEIGHT,
                             SNAPSHT_TILE_WIDTH,
                             SNPSHT_SCHD_MESSAGE_TEXTBOX,
                             this,
                             snapSchdStrings[SNPSHT_SCHD_MESSAGE_TEXTBOX],
                             COMMON_LAYER,
                             true,
                             0,
                             SCALE_WIDTH(300),
                             QRegExp(""),
                             false,
                             false,
                             false,
                             LEFT_MARGIN_FROM_CENTER);

    m_elementList[SNPSHT_SCHD_MESSAGE_TEXTBOX] = msgBox;

    connect (msgBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));


    QMap<quint8, QString> imageRateMapList;
    imageRateMapList.clear ();

    for(quint8 index = 0; index < 5; index++)
    {
        imageRateMapList.insert (index,QString("%1").arg (index + 1));
    }

    imageRateDropDownBox = new DropDown(cameraListDropDownBox->x (),
                                        msgBox->y () + 2*BGTILE_HEIGHT,
                                        SNAPSHT_TILE_WIDTH,
                                        BGTILE_HEIGHT,
                                        SNPSHT_SCHD_MAX_IMAGE_RATE_DROPBOX,
                                        DROPDOWNBOX_SIZE_90,
                                        snapSchdStrings[SNPSHT_SCHD_MAX_IMAGE_RATE_DROPBOX],
                                        imageRateMapList,
                                        this,
                                        "",
                                        true,
                                        0,
                                        COMMON_LAYER,
                                        true,
                                        4,
                                        false,
                                        false,
                                        5,
                                        LEFT_MARGIN_FROM_CENTER);

    m_elementList[SNPSHT_SCHD_MAX_IMAGE_RATE_DROPBOX] = imageRateDropDownBox;

    connect (imageRateDropDownBox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotDropDownValueChange(QString,quint32)));

    connect (imageRateDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    schdSetBtn = new PageOpenButton(cameraListDropDownBox->x () ,
                                    imageRateDropDownBox->y () + imageRateDropDownBox->height (),
                                    SNAPSHT_TILE_WIDTH,
                                    SCALE_HEIGHT(30),
                                    SNPSHT_SCHD_SET_BUTTON,
                                    PAGEOPENBUTTON_SMALL,
                                    "Set",
                                    this,
                                    snapSchdStrings[SNPSHT_SCHD_SET_BUTTON],
                                    "",
                                    true,
                                    SCALE_WIDTH(255),
                                    COMMON_LAYER, true,
                                    ALIGN_START_X_CENTRE_Y,
                                    LEFT_MARGIN_FROM_CENTER);

    m_elementList[SNPSHT_SCHD_SET_BUTTON] = schdSetBtn;

    connect (schdSetBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotPageOpenBtnClick(int)));

    connect (schdSetBtn,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    QString emailNote = QString("Note : Multiple Email IDs can be added using (;) or (,)");

    emailNoteLabel = new TextLabel(schdSetBtn->x(),
                                   schdSetBtn->y() + (BGTILE_HEIGHT),
                                   NORMAL_FONT_SIZE,
                                   emailNote,
                                   this);

    if(uploadListDropDownBox->getCurrValue() == "Email Server")
    {
        emailNoteLabel->setVisible(true);
    }
    else
    {
        emailNoteLabel->setVisible(false);
    }

}

void SnapshotSchedule::getConfig ()
{
    getCameraList ();
    createPayload (MSG_GET_CFG);
}

void SnapshotSchedule::defaultConfig ()
{
    createPayload (MSG_DEF_CFG);
}

void SnapshotSchedule::saveConfig ()
{
    if((enableSnapshotCheckBox->getCurrentState () == ON_STATE))
    {
        if ( uploadListDropDownBox->getIndexofCurrElement() == 0)
        {
            if ( (!emailTextBox->doneKeyValidation()))
            {
                return;
            }
        }
    }

    if(saveConfigFeilds())
        createPayload (MSG_SET_CFG);
}

void SnapshotSchedule::createPayload(REQ_MSG_ID_e msgType )
{
    QString payloadString = payloadLib->createDevCnfgPayload(msgType,
                                                             SNAPSHOT_SETTING_TABLE_INDEX,
                                                             currentCameraIndex,
                                                             currentCameraIndex,
                                                             CNFG_FRM_INDEX,
                                                             SNAPSHT_CONFIG_MAX_FIELDS,
                                                             SNAPSHT_CONFIG_MAX_FIELDS);

    payloadString =  payloadLib->createDevCnfgPayload(msgType,
                                                      SNAPSHOT_SCHEDULE_SETTING_TABLE_INDEX,
                                                      currentCameraIndex,
                                                      currentCameraIndex,
                                                      CNFG_FRM_INDEX,
                                                      SNAPSHT_SCHD_CONFIG_MAX_FIELDS,
                                                      SNAPSHT_SCHD_CONFIG_MAX_FIELDS,
                                                      payloadString,
                                                      SNAPSHT_CONFIG_MAX_FIELDS);

    DevCommParam* param = new DevCommParam();
    param->msgType = msgType;
    param->payload = payloadString;

    processBar->loadProcessBar ();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

bool SnapshotSchedule::saveConfigFeilds()
{
    if((uploadListDropDownBox->getCurrValue () == uploadServers.at (0)) &&
            (enableSnapshotCheckBox->getCurrentState () == ON_STATE))
    {
        if(emailTextBox->getInputText () == "")
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_EMAIL_ADD));
            cameraListDropDownBox->setIndexofCurrElement (currentCameraIndex -1);
            return false;
        }
        else if (subjectTextBox->getInputText () == "")
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_SUBJECT));
            cameraListDropDownBox->setIndexofCurrElement (currentCameraIndex-1);
            return false;
        }
        else if (msgBox->getInputText () == "")
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_MESS));
            cameraListDropDownBox->setIndexofCurrElement (currentCameraIndex-1);
            return false;
        }
    }

    QString stopTime = "";
    QString startTime = "";

    payloadLib->setCnfgArrayAtIndex (SNPSHT_SCHD_FIELDS_ENB_SCHD, enableSnapshotCheckBox->getCurrentState());
    payloadLib->setCnfgArrayAtIndex (SNPSHT_SCHD_FIELDS_UPLD_LOCATION, uploadListDropDownBox->getIndexofCurrElement());
    payloadLib->setCnfgArrayAtIndex (SNPSHT_SCHD_FIELDS_EMAIL_ADDRESS, emailTextBox->getInputText());
    payloadLib->setCnfgArrayAtIndex (SNPSHT_SCHD_FIELDS_SUBJECT, subjectTextBox->getInputText ());
    payloadLib->setCnfgArrayAtIndex (SNPSHT_SCHD_FIELDS_MESSAGE, msgBox->getInputText ());
    payloadLib->setCnfgArrayAtIndex (SNPSHT_SCHD_FIELDS_IMAGES_PER_MIN, imageRateDropDownBox->getCurrValue());

    SET_CAMERA_MASK_BIT(copyToCameraFields, (currentCameraIndex - 1));
    for (quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
    {
        payloadLib->setCnfgArrayAtIndex (SNPSHT_SCHD_FIELDS_COPY_TO_CAM_START + maskIdx, copyToCameraFields.bitMask[maskIdx]);
        payloadLib->setCnfgArrayAtIndex (SNPSHT_SCHD_FIELDS_SCHD_COPY_TO_CAM_START + maskIdx, copyToCameraFields.bitMask[maskIdx]);
    }

    payloadLib->setCnfgArrayAtIndex (SNPSHT_SCHD_FIELDS_ENTRDAY,((isScheduleSetForEntireDay & IS_SET_SCHD_ENTIREDAY_CHECKED) ? 1 : 0));

    if (true == (isScheduleSetForEntireDay & IS_SET_SCHD_ENTIREDAY_CHECKED))
    {
        memset(&scheduleTimeing, 0, sizeof(scheduleTimeing));
    }

    for(quint8 index1 = 0 ; index1 < MAX_SCHD_TIME_SLOT ;index1++)
    {
        if (scheduleTimeing.start_hour[index1] > 24)
            scheduleTimeing.start_hour[index1] = 0;

        (scheduleTimeing.start_hour[index1] < 10) ?
                startTime = ( QString("0") + QString("%1").arg(scheduleTimeing.start_hour[index1])) :
                startTime = ( QString("%1").arg(scheduleTimeing.start_hour[index1]));

        if (scheduleTimeing.start_min[index1] > 60)
            scheduleTimeing.start_min[index1] = 0;

        (scheduleTimeing.start_min[index1] < 10) ?
                    startTime.append(QString("0") + QString("%1").arg(scheduleTimeing.start_min[index1])) :
                    startTime.append(QString("%1").arg(scheduleTimeing.start_min[index1]));

        payloadLib->setCnfgArrayAtIndex((SNPSHT_SCHD_FIELDS_START_TIME1 + (index1*2)),startTime);

        if (scheduleTimeing.stop_hour[index1] > 24)
            scheduleTimeing.stop_hour[index1] = 0;

        (scheduleTimeing.stop_hour[index1] < 10) ?
                    stopTime =(QString("0") + QString("%1").arg (scheduleTimeing.stop_hour[index1])) :
                stopTime =( QString("%1").arg (scheduleTimeing.stop_hour[index1]) );

        if (scheduleTimeing.stop_min[index1] > 60)
            scheduleTimeing.stop_min[index1] = 0;

        (scheduleTimeing.stop_min[index1] < 10) ?
                    stopTime.append (QString("0") + QString("%1").arg(scheduleTimeing.stop_min[index1])) :
                    stopTime.append (QString("%1").arg(scheduleTimeing.stop_min[index1]));

        payloadLib->setCnfgArrayAtIndex((SNPSHT_SCHD_FIELDS_END_TIME1 + (index1*2)),stopTime);
    }

    payloadLib->setCnfgArrayAtIndex (SNPSHT_SCHD_FIELDS_COPY_WEEKDAYS,copyToWeekdaysFields);

    return true;
}

void SnapshotSchedule::processDeviceResponse (DevCommParam *param, QString deviceName)
{
    bool isProcessBarUnload = true;

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
                if((payloadLib->getcnfgTableIndex (0) == SNAPSHOT_SETTING_TABLE_INDEX)
                        && (payloadLib->getcnfgTableIndex (1) == SNAPSHOT_SCHEDULE_SETTING_TABLE_INDEX))
                {
                    quint32 temp;
                    QString tempStr;
                    m_configResponse.clear();

                    enableSnapshotCheckBox->changeState((OPTION_STATE_TYPE_e)payloadLib->getCnfgArrayAtIndex (SNPSHT_SCHD_FIELDS_ENB_SCHD).toUInt ());
                    m_configResponse[SNPSHT_SCHD_FIELDS_ENB_SCHD]=payloadLib->getCnfgArrayAtIndex (SNPSHT_SCHD_FIELDS_ENB_SCHD);

                    temp = payloadLib->getCnfgArrayAtIndex (SNPSHT_SCHD_FIELDS_UPLD_LOCATION).toUInt ();
                    m_configResponse[SNPSHT_SCHD_FIELDS_UPLD_LOCATION]=payloadLib->getCnfgArrayAtIndex (SNPSHT_SCHD_FIELDS_UPLD_LOCATION);

                    uploadListDropDownBox->setIndexofCurrElement (temp);

                    slotCheckboxClicked(enableSnapshotCheckBox->getCurrentState (),SNPSHT_SCHD_ENBL_SCHD);

                    tempStr = payloadLib->getCnfgArrayAtIndex (SNPSHT_SCHD_FIELDS_SUBJECT).toString ();
                    m_configResponse[SNPSHT_SCHD_FIELDS_SUBJECT]=payloadLib->getCnfgArrayAtIndex (SNPSHT_SCHD_FIELDS_SUBJECT);

                    subjectTextBox->setInputText (tempStr);

                    tempStr = payloadLib->getCnfgArrayAtIndex (SNPSHT_SCHD_FIELDS_EMAIL_ADDRESS).toString ();
                    emailTextBox->setInputText (tempStr);
                    m_configResponse[SNPSHT_SCHD_FIELDS_EMAIL_ADDRESS]= payloadLib->getCnfgArrayAtIndex (SNPSHT_SCHD_FIELDS_EMAIL_ADDRESS);

                    tempStr = payloadLib->getCnfgArrayAtIndex (SNPSHT_SCHD_FIELDS_MESSAGE).toString ();
                    msgBox->setInputText (tempStr);
                    m_configResponse[SNPSHT_SCHD_FIELDS_MESSAGE]= payloadLib->getCnfgArrayAtIndex (SNPSHT_SCHD_FIELDS_MESSAGE);

                    tempStr = payloadLib->getCnfgArrayAtIndex (SNPSHT_SCHD_FIELDS_IMAGES_PER_MIN).toString ();
                    imageRateDropDownBox->setCurrValue (tempStr);
                    m_configResponse[SNPSHT_SCHD_FIELDS_IMAGES_PER_MIN]= payloadLib->getCnfgArrayAtIndex (SNPSHT_SCHD_FIELDS_IMAGES_PER_MIN);

                    for (quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
                    {
                        copyToCameraFields.bitMask[maskIdx] = payloadLib->getCnfgArrayAtIndex(SNPSHT_SCHD_FIELDS_COPY_TO_CAM_START + maskIdx).toULongLong();
                        m_configResponse[SNPSHT_SCHD_FIELDS_COPY_TO_CAM_START + maskIdx] = copyToCameraFields.bitMask[maskIdx];
                    }

                    isScheduleSetForEntireDay =  payloadLib->getCnfgArrayAtIndex (SNPSHT_SCHD_FIELDS_ENTRDAY).toBool ();
                    m_configResponse[SNPSHT_SCHD_FIELDS_ENTRDAY]= isScheduleSetForEntireDay;

                    for(quint8 index= 0; index < MAX_SCHD_TIME_SLOT; index++)
                    {

                        m_configResponse[SNPSHT_SCHD_FIELDS_START_TIME1 + index*2]= payloadLib->getCnfgArrayAtIndex (SNPSHT_SCHD_FIELDS_START_TIME1 + index*2);
                        m_configResponse[SNPSHT_SCHD_FIELDS_END_TIME1 + index*2]= payloadLib->getCnfgArrayAtIndex (SNPSHT_SCHD_FIELDS_END_TIME1 + index*2);
                        tempStr = payloadLib->getCnfgArrayAtIndex (SNPSHT_SCHD_FIELDS_START_TIME1 + index*2).toString ().mid (0,2);
                        scheduleTimeing.start_hour[index] = tempStr.toUInt ();

                        tempStr = payloadLib->getCnfgArrayAtIndex (SNPSHT_SCHD_FIELDS_START_TIME1 + index*2).toString ().mid (2,2);
                        scheduleTimeing.start_min[index] = tempStr.toUInt ();

                        tempStr = payloadLib->getCnfgArrayAtIndex (SNPSHT_SCHD_FIELDS_END_TIME1 + index*2).toString ().mid (0,2);
                        scheduleTimeing.stop_hour[index] = tempStr.toUInt ();

                        tempStr = payloadLib->getCnfgArrayAtIndex (SNPSHT_SCHD_FIELDS_END_TIME1 + index*2).toString ().mid (2,2);
                        scheduleTimeing.stop_min[index] = tempStr.toUInt ();

                    }

                    copyToWeekdaysFields = payloadLib->getCnfgArrayAtIndex (SNPSHT_SCHD_FIELDS_COPY_WEEKDAYS).toUInt ();
                    m_configResponse[SNPSHT_SCHD_FIELDS_COPY_WEEKDAYS]=copyToWeekdaysFields;
                }
            }
                break;

            case MSG_SET_CFG:
            {
                isProcessBarUnload = false;
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
                currentCameraIndex = (cameraListDropDownBox->getIndexofCurrElement () + 1);
                getConfig ();
            }
                break;

            case MSG_DEF_CFG:
            {
                isProcessBarUnload = false;
                currentCameraIndex = (cameraListDropDownBox->getIndexofCurrElement () + 1);
                getConfig ();
            }
                break;

            default:
                break;

            }
        }
            break;

        default:
            processBar->unloadProcessBar ();
            isProcessBarUnload = false;
            infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
			cameraListDropDownBox->setIndexofCurrElement (currentCameraIndex-1);
        }
    }

    if(isProcessBarUnload == true)
    {
        processBar->unloadProcessBar ();
    }
}

void SnapshotSchedule::getCameraList ()
{
    QString tempStr;

    cameraList.clear();

    for(quint8 index = 0; index < devTableInfo->totalCams; index++)
    {
        tempStr = applController->GetCameraNameOfDevice (currDevName,index);

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

    cameraListDropDownBox->setNewList (cameraList,(currentCameraIndex -1));
}

void SnapshotSchedule::slotPageOpenBtnClick (int index )
{
    switch (index)
    {
    case SNPSHT_SCHD_COPYTOCAM:
        if(copytoCamera != NULL)
        {
            return;
        }

        memset(&copyToCameraFields, 0, sizeof(copyToCameraFields));
        SET_CAMERA_MASK_BIT(copyToCameraFields, (currentCameraIndex - 1));

        for (quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
        {
            m_configResponse[SNPSHT_SCHD_FIELDS_COPY_TO_CAM_START + maskIdx] = copyToCameraFields.bitMask[maskIdx];
        }

        copytoCamera = new CopyToCamera(cameraList,
                                        copyToCameraFields,
                                        parentWidget(),
                                        "Schedule Snapshot",
                                        SNPSHT_SCHD_COPYTOCAM);
        connect (copytoCamera,
                 SIGNAL(sigDeleteObject(quint8)),
                 this,
                 SLOT(slotCopytoCamDelete(quint8)));
        break;

    case SNPSHT_SCHD_SET_BUTTON:

        if(setSchedule != NULL)
        {
            return;
        }

        setSchedule = new SetSchedule(&scheduleTimeing,
                                      &isScheduleSetForEntireDay,
                                      &copyToWeekdaysFields,
                                      parentWidget(),
                                      0,
                                      SET_SCHD_SNAP_SHOT);

        connect (setSchedule,
                 SIGNAL(sigDeleteObject()),
                 this,
                 SLOT(slotSchdObjectDelete()));
        break;

    default:
        break;
    }
}

void SnapshotSchedule:: slotCopytoCamDelete(quint8)
{
    if(copytoCamera != NULL)
    {
        disconnect (copytoCamera,
                    SIGNAL(sigDeleteObject(quint8)),
                    this,
                    SLOT(slotCopytoCamDelete(quint8)));
        delete copytoCamera;
        copytoCamera = NULL;
    }

    if(IS_VALID_OBJ(m_elementList[m_currentElement]))
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

void SnapshotSchedule::slotSchdObjectDelete ()
{
    if(setSchedule != NULL)
    {
        disconnect (setSchedule,
                    SIGNAL(sigDeleteObject()),
                    this,
                    SLOT(slotSchdObjectDelete()));
        delete setSchedule;
        setSchedule = NULL;
    }
    if(IS_VALID_OBJ(m_elementList[m_currentElement]))
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

void SnapshotSchedule::slotCheckboxClicked(OPTION_STATE_TYPE_e state,int)
{
    bool isEnable = (state == ON_STATE) ? true : false;
    QString str = uploadListDropDownBox->getCurrValue ();

    uploadListDropDownBox->setIsEnabled (isEnable);
    msgBox->setIsEnabled ((str == uploadServers.at (0)) ? isEnable : false);
    emailTextBox->setIsEnabled ((str == uploadServers.at (0)) ? isEnable : false);
    subjectTextBox->setIsEnabled ((str == uploadServers.at (0)) ? isEnable : false);
    imageRateDropDownBox->setIsEnabled (isEnable);
    schdSetBtn->setIsEnabled (isEnable);
}

bool SnapshotSchedule::isUserChangeConfig()
{
    if(m_configResponse.isEmpty())
    {
        return false;
    }

    if((IS_VALID_OBJ(enableSnapshotCheckBox)) && (m_configResponse[SNPSHT_SCHD_FIELDS_ENB_SCHD] != enableSnapshotCheckBox->getCurrentState()))
    {
        return true;
    }

    if((IS_VALID_OBJ(uploadListDropDownBox)) && (m_configResponse[SNPSHT_SCHD_FIELDS_UPLD_LOCATION] != uploadListDropDownBox->getIndexofCurrElement()))
    {
        return true;
    }

    if((IS_VALID_OBJ(emailTextBox)) && (m_configResponse[SNPSHT_SCHD_FIELDS_EMAIL_ADDRESS] != emailTextBox->getInputText()))
    {
        return true;
    }

    if((IS_VALID_OBJ(subjectTextBox)) && (m_configResponse[SNPSHT_SCHD_FIELDS_SUBJECT] != subjectTextBox->getInputText()))
    {
        return true;
    }

    if((IS_VALID_OBJ(imageRateDropDownBox)) && (m_configResponse[SNPSHT_SCHD_FIELDS_IMAGES_PER_MIN] != imageRateDropDownBox->getCurrValue()))
    {
        return true;
    }

    if((IS_VALID_OBJ(msgBox)) && (m_configResponse[SNPSHT_SCHD_FIELDS_MESSAGE] != msgBox->getInputText()))
    {
        return true;
    }

    if(m_configResponse[SNPSHT_SCHD_FIELDS_ENTRDAY] != (isScheduleSetForEntireDay & IS_SET_SCHD_ENTIREDAY_CHECKED))
    {
        return true;
    }

    for(quint8 index1 = 0; index1 < MAX_SCHD_TIME_SLOT; index1++)
    {
        QString startTime = "";
        QString stopTime = "";

        if(scheduleTimeing.start_hour[index1] > 24)
            scheduleTimeing.start_hour[index1] = 0;

        startTime = (QString("%1").arg (scheduleTimeing.start_hour[index1]).rightJustified(2,'0'));

        if(scheduleTimeing.start_min[index1] > 60)
            scheduleTimeing.start_min[index1] = 0;

        startTime.append(QString("%1").arg(scheduleTimeing.start_min[index1]).rightJustified(2,'0'));

        if(m_configResponse[SNPSHT_SCHD_FIELDS_START_TIME1 + (index1*2)] != startTime)
        {
            return true;
        }

        if(scheduleTimeing.stop_hour[index1] > 24)
            scheduleTimeing.stop_hour[index1] = 0;

        stopTime =(QString("%1").arg (scheduleTimeing.stop_hour[index1]).rightJustified(2,'0'));

        if(scheduleTimeing.stop_min[index1] > 60)
            scheduleTimeing.stop_min[index1] = 0;

        stopTime.append(QString("%1").arg(scheduleTimeing.stop_min[index1]).rightJustified(2,'0'));

        if(m_configResponse[SNPSHT_SCHD_FIELDS_END_TIME1 + (index1*2)] != stopTime)
        {
            return true;
        }
    }

    if(m_configResponse[SNPSHT_SCHD_FIELDS_COPY_WEEKDAYS] != copyToWeekdaysFields)
    {
        return true;
    }

    for (quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
    {
        if( copyToCameraFields.bitMask[maskIdx] != m_configResponse[SNPSHT_SCHD_FIELDS_COPY_TO_CAM_START + maskIdx])
        {
            return true;
        }
    }

    return false;
}

void SnapshotSchedule::handleInfoPageMessage(int index)
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
        if(infoPage->getText() == (ValidationMessage::getValidationMessage(SAVE_CHANGES)))
        {
            currentCameraIndex = (cameraListDropDownBox->getIndexofCurrElement () + 1);
            getConfig();
        }
    }
}


void SnapshotSchedule::slotDropDownValueChange (QString str , quint32 index)
{
    switch(index)
    {
    case  SNPSHT_SCHD_UPLOAD_DROPBOX :
    {
        if(str == uploadServers.at (0))
        {
            msgBox->setIsEnabled (true);
            emailTextBox->setIsEnabled (true);
            subjectTextBox->setIsEnabled (true);
            emailNoteLabel->setVisible(true);
        }
        else
        {
            msgBox->setIsEnabled (false);
            emailTextBox->setIsEnabled (false);
            subjectTextBox->setIsEnabled (false);
            emailNoteLabel->setVisible(false);
        }
    }
        break;

    case SNPSHT_SCHD_CAMLIST_DROPBOX:
    {
        if(isUserChangeConfig())
        {
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(SAVE_CHANGES),true,false,"",CONFORMATION_BTN_YES,CONFORMATION_BTN_NO);
        }
        else
        {
            currentCameraIndex = (cameraListDropDownBox->getIndexofCurrElement () + 1);
            getConfig ();
        }
    }
        break;

    default:
        break;
    }
}

void SnapshotSchedule::slotTextBoxInfoPage(int index ,INFO_MSG_TYPE_e msgType)
{
    if(index == SNPSHT_SCHD_EMAIL_TEXTBOX)
    {
        if(msgType == INFO_MSG_ERROR)
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_VAILD_EMAIL_ADD));
        }
        else if(msgType == INFO_MSG_STRAT_CHAR)
        {
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(ENT_FIRST_ALPH));
        }
    }
}
