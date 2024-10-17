#include "StorageManagment.h"
#include "ValidationMessage.h"

#define ELE_MARGIN SCALE_WIDTH(50)
#define ELE_HEADING_MARGIN SCALE_WIDTH(15)

typedef enum{

    STR_ALRT_STOP_REC,
    STR_OVERWRT_REC,
    STR_CLN_REC,
    STR_CLN_REC_TXTBX,
    STR_REC_RTNT,
    STR_REC_DRV_CTRL,
    STR_REC_DRV_TXTBX,
    STR_REC_CAMWISE,
    STR_REC_CAM_SEL,
    STR_BCK_RTNT,
    STR_BCK_CAM_SEL,
    STR_STOR_ALERT,
    STR_STOR_ALERT_TXTBX,

    MAX_STR_MNG_CTRL
}STR_MNG_CTRL_e;


typedef enum{

    STR_RTET_MNG_HEADING = 4,
    STR_REC_RTET_MNG,
    STR_REC_DRV,
    STR_REC_RTER_CAMWISE,
    STR_SEL_CAM,
    STR_REC_BCKUP_RTET_MNG,
    STR_STOR_ALRT,

    MAX_STR_MNG_STR
}STR_MNG_STR_e;

static const QString storageManagmentStrings[MAX_STR_MNG_STR] = {
    "Storage Full",
    "Alert and stop recording",
    "Overwrite oldest file",
    "Clean oldest file and create storage space of",
    "Retention Management",
    "Recording Retention",
    "Recording Drive",
    "Camera Wise",
    "Select Camera",
    "Backup Retention",
    "Storage alert when remaining space is less than",
};

typedef enum {

    STR_HDD_ACT_MODE,
    STR_HDD_CLN_SPEC,
    STR_ENB_REG_CLN,
    STR_REC_ENBL_REC_RETN,
    STR_REC_DRV_RETN_TIME,
    STR_REC_STOR_ENB,
    STR_REC_STOR_MEM,
    STR_BCK_RET_ENB,
    STR_REC_RETN_TIME_CAM1,
    STR_BCK_RETN_TIME_CAM1= (STR_REC_RETN_TIME_CAM1 + MAX_CAMERAS),

    MAX_STR_MNG_FEILDS =(STR_BCK_RETN_TIME_CAM1 + MAX_CAMERAS)
}STR_MNG_FEILDS_e;

StorageManagment::StorageManagment(QString deviceName, QWidget *parent,DEV_TABLE_INFO_t *devTabInfo)
    : ConfigPageControl(deviceName, parent,MAX_STR_MNG_CTRL,devTabInfo)
{
    createDefaultComponents ();
    getCameraList();
    StorageManagment::getConfig();
}

void StorageManagment:: createDefaultComponents ()
{
    backupRetSelCam = NULL;
    recordRetSelCam = NULL;

    elementHeading[0] = new ElementHeading((SCALE_WIDTH(PAGE_RIGHT_PANEL_WIDTH) - BGTILE_LARGE_SIZE_WIDTH)/2 + SCALE_WIDTH(5) ,
                                           SCALE_HEIGHT(65),
                                           BGTILE_LARGE_SIZE_WIDTH,
                                           BGTILE_HEIGHT,
                                           storageManagmentStrings[0],
                                           TOP_LAYER,
                                           this,
                                           false,
                                           ELE_HEADING_MARGIN, NORMAL_FONT_SIZE, true);

    for(quint8 index = 0 ; index < MAX_STR_MNG_STORG_MODE ; index++ )
    {
        storageOptions[index] = new OptionSelectButton(elementHeading[0]->x (),
                                                       elementHeading[0]->y () +
                                                       elementHeading[0]->height () +
                                                       BGTILE_HEIGHT*index,
                                                       BGTILE_LARGE_SIZE_WIDTH,
                                                       BGTILE_HEIGHT,
                                                       RADIO_BUTTON_INDEX,
                                                       storageManagmentStrings[index + 1],
                                                       this,
                                                       index < 2 ?
                                                           MIDDLE_TABLE_LAYER :
                                                           BOTTOM_TABLE_LAYER,
                                                       ELE_MARGIN,
                                                       MX_OPTION_TEXT_TYPE_SUFFIX,
                                                       NORMAL_FONT_SIZE,
                                                       STR_ALRT_STOP_REC + index, true, NORMAL_FONT_COLOR,
                                                       index < 2 ? true : false);

        m_elementList[STR_ALRT_STOP_REC + index] =  storageOptions[index];

        connect (storageOptions[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));

        connect (storageOptions[index],
                 SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                 this,
                 SLOT(slotSelectionButtonClick(OPTION_STATE_TYPE_e,int)));
    }

    storageTextBoxParam = new TextboxParam ();
    storageTextBoxParam->suffixStr = "(5 - 90%)";
    storageTextBoxParam->isNumEntry = true;
    storageTextBoxParam->minNumValue = 5;
    storageTextBoxParam->maxNumValue = 90;
    storageTextBoxParam->maxChar = 2;
    storageTextBoxParam->validation = QRegExp(QString("[0-9]"));

    storageTextBox = new TextBox(storageOptions[2]->x () + BGTILE_LARGE_SIZE_WIDTH/2,
                                 storageOptions[2]->y (),
                                 BGTILE_LARGE_SIZE_WIDTH,
                                 BGTILE_HEIGHT,
                                 STR_CLN_REC_TXTBX,
                                 TEXTBOX_EXTRASMALL,
                                 this,
                                 storageTextBoxParam,
                                 NO_LAYER,
                                 false);

    m_elementList[STR_CLN_REC_TXTBX] = storageTextBox;

    connect (storageTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    connect (storageTextBox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotLoadInfoPage(int,INFO_MSG_TYPE_e)));

    elementHeading[1] = new ElementHeading(storageOptions[2]->x (),
                                           storageOptions[2]->y () +  storageOptions[2]->height () + SCALE_HEIGHT(5),
                                           BGTILE_LARGE_SIZE_WIDTH,
                                           BGTILE_HEIGHT,
                                           storageManagmentStrings[STR_RTET_MNG_HEADING],
                                           TOP_LAYER,
                                           this,
                                           false,
                                           ELE_HEADING_MARGIN, NORMAL_FONT_SIZE, true);

    recordingRetation = new OptionSelectButton( elementHeading[1]->x (),
                                                elementHeading[1]->y () +  elementHeading[1]->height (),
                                                BGTILE_LARGE_SIZE_WIDTH,
                                                BGTILE_HEIGHT,
                                                CHECK_BUTTON_INDEX,
                                                storageManagmentStrings[STR_REC_RTET_MNG],
                                                this,
                                                MIDDLE_TABLE_LAYER,
                                                ELE_MARGIN,
                                                MX_OPTION_TEXT_TYPE_SUFFIX,
                                                NORMAL_FONT_SIZE,
                                                STR_REC_RTNT);

    m_elementList[STR_REC_RTNT] = recordingRetation;

    connect (recordingRetation,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    connect (recordingRetation,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotSelectionButtonClick(OPTION_STATE_TYPE_e,int)));

    recordingDrive = new OptionSelectButton( elementHeading[1]->x () + SCALE_WIDTH(300),
                                             recordingRetation->y (),
                                             BGTILE_LARGE_SIZE_WIDTH,
                                             BGTILE_HEIGHT,
                                             RADIO_BUTTON_INDEX,
                                             storageManagmentStrings[STR_REC_DRV],
                                             this,
                                             NO_LAYER,
                                             ELE_MARGIN,
                                             MX_OPTION_TEXT_TYPE_SUFFIX,
                                             NORMAL_FONT_SIZE,
                                             STR_REC_DRV_CTRL,
                                             false);

    m_elementList[STR_REC_DRV_CTRL] = recordingDrive;

    connect (recordingDrive,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    connect (recordingDrive,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotSelectionButtonClick(OPTION_STATE_TYPE_e,int)));

    recordingTextBoxParam = new TextboxParam ();
    recordingTextBoxParam->suffixStr = "(1 - 60 days)";
    recordingTextBoxParam->isNumEntry = true;
    recordingTextBoxParam->minNumValue = 1;
    recordingTextBoxParam->maxNumValue = 60;
    recordingTextBoxParam->maxChar = 2;
    recordingTextBoxParam->validation = QRegExp(QString("[0-9]"));

    recordingTextBox = new TextBox(storageOptions[2]->x () + BGTILE_LARGE_SIZE_WIDTH/2 ,
                                   recordingRetation->y (),
                                   BGTILE_LARGE_SIZE_WIDTH,
                                   BGTILE_HEIGHT,
                                   STR_REC_DRV_TXTBX,
                                   TEXTBOX_EXTRASMALL,
                                   this,
                                   recordingTextBoxParam,
                                   NO_LAYER,
                                   false);

    m_elementList[STR_REC_DRV_TXTBX] = recordingTextBox;

    connect (recordingTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    connect (recordingTextBox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotLoadInfoPage(int,INFO_MSG_TYPE_e)));

    recordingRetationCameraWise = new OptionSelectButton( recordingRetation->x (),
                                                          recordingRetation->y () +
                                                          recordingRetation->height (),
                                                          BGTILE_LARGE_SIZE_WIDTH,
                                                          BGTILE_HEIGHT,
                                                          RADIO_BUTTON_INDEX,
                                                          storageManagmentStrings
                                                          [STR_REC_RTER_CAMWISE],
                                                          this,
                                                          MIDDLE_TABLE_LAYER,
                                                          SCALE_WIDTH(290),
                                                          MX_OPTION_TEXT_TYPE_SUFFIX,
                                                          NORMAL_FONT_SIZE,
                                                          STR_REC_CAMWISE,
                                                          false);

    m_elementList[STR_REC_CAMWISE] = recordingRetationCameraWise;

    connect (recordingRetationCameraWise,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    connect (recordingRetationCameraWise,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotSelectionButtonClick(OPTION_STATE_TYPE_e,int)));

    recordingSelectCamera = new PageOpenButton( recordingRetationCameraWise->x () +
                                                BGTILE_LARGE_SIZE_WIDTH/2  - SCALE_WIDTH(10),
                                                recordingRetationCameraWise->y (),
                                                BGTILE_LARGE_SIZE_WIDTH,
                                                BGTILE_HEIGHT,
                                                STR_REC_CAM_SEL,
                                                PAGEOPENBUTTON_EXTRALARGE,
                                                storageManagmentStrings[STR_SEL_CAM],
                                                this,
                                                "","",
                                                false,
                                                0,
                                                NO_LAYER,
                                                false,
                                                ALIGN_CENTRE_X_CENTER_Y);

    m_elementList[STR_REC_CAM_SEL] = recordingSelectCamera;

    connect (recordingSelectCamera,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    connect (recordingSelectCamera,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    backupRetation = new OptionSelectButton( recordingRetationCameraWise->x (),
                                             recordingRetationCameraWise->y () +
                                             recordingRetationCameraWise->height (),
                                             BGTILE_LARGE_SIZE_WIDTH,
                                             BGTILE_HEIGHT,
                                             CHECK_BUTTON_INDEX,
                                             storageManagmentStrings[STR_REC_BCKUP_RTET_MNG],
                                             this,
                                             BOTTOM_TABLE_LAYER,
                                             ELE_MARGIN,
                                             MX_OPTION_TEXT_TYPE_SUFFIX,
                                             NORMAL_FONT_SIZE,
                                             STR_BCK_RTNT);

    m_elementList[STR_BCK_RTNT] = backupRetation;

    connect (backupRetation,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    connect (backupRetation,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotSelectionButtonClick(OPTION_STATE_TYPE_e,int)));

    backupSelectCamera = new PageOpenButton( recordingRetationCameraWise->x () +
                                             SCALE_WIDTH(260) ,
                                             backupRetation->y (),
                                             BGTILE_LARGE_SIZE_WIDTH,
                                             BGTILE_HEIGHT,
                                             STR_BCK_CAM_SEL,
                                             PAGEOPENBUTTON_EXTRALARGE,
                                             storageManagmentStrings[STR_SEL_CAM],
                                             this,
                                             "","",
                                             false,
                                             0,
                                             NO_LAYER,
                                             false,
                                             ALIGN_CENTRE_X_CENTER_Y);

    m_elementList[STR_BCK_CAM_SEL] = backupSelectCamera;

    connect (backupSelectCamera,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    connect (backupSelectCamera,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    storageAlert = new OptionSelectButton( backupRetation->x (),
                                           backupRetation->y () +
                                           backupRetation->height () + SCALE_HEIGHT(5),
                                           BGTILE_LARGE_SIZE_WIDTH,
                                           BGTILE_HEIGHT,
                                           CHECK_BUTTON_INDEX,
                                           storageManagmentStrings[STR_STOR_ALRT],
                                           this,
                                           COMMON_LAYER,
                                           ELE_HEADING_MARGIN,
                                           MX_OPTION_TEXT_TYPE_SUFFIX,
                                           NORMAL_FONT_SIZE,
                                           STR_STOR_ALERT);

    m_elementList[STR_STOR_ALERT] = storageAlert;

    connect (storageAlert,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    connect (storageAlert,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotSelectionButtonClick(OPTION_STATE_TYPE_e,int)));

    storageAlertTextBoxParam = new TextboxParam ();
    storageAlertTextBoxParam->suffixStr = "(1 - 99 GB)";
    storageAlertTextBoxParam->isNumEntry = true;
    storageAlertTextBoxParam->minNumValue = 1;
    storageAlertTextBoxParam->maxNumValue = 99;
    storageAlertTextBoxParam->maxChar = 2;
    storageAlertTextBoxParam->validation = QRegExp(QString("[0-9]"));

    storageAlertTextBox = new TextBox(storageOptions[2]->x () + BGTILE_LARGE_SIZE_WIDTH/2 - SCALE_WIDTH(30),
                                      storageAlert->y (),
                                      BGTILE_LARGE_SIZE_WIDTH,
                                      BGTILE_HEIGHT,
                                      STR_STOR_ALERT_TXTBX,
                                      TEXTBOX_EXTRASMALL,
                                      this,
                                      storageAlertTextBoxParam,
                                      NO_LAYER);

    m_elementList[STR_STOR_ALERT_TXTBX] = storageAlertTextBox;

    connect (storageAlertTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    connect (storageAlertTextBox,
             SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
             this,
             SLOT(slotLoadInfoPage(int,INFO_MSG_TYPE_e)));

}

StorageManagment::~StorageManagment()
{
    delete elementHeading[0];

    for(quint8 index = 0 ; index < MAX_STR_MNG_STORG_MODE ; index++ )
    {
        disconnect (storageOptions[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));

        disconnect (storageOptions[index],
                    SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                    this,
                    SLOT(slotSelectionButtonClick(OPTION_STATE_TYPE_e,int)));

        delete storageOptions[index];
    }

    disconnect (storageTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

    disconnect (storageTextBox,
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotLoadInfoPage(int,INFO_MSG_TYPE_e)));

    delete storageTextBox;
    delete storageTextBoxParam;

    delete elementHeading[1];

    disconnect (recordingRetation,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

    disconnect (recordingRetation,
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotSelectionButtonClick(OPTION_STATE_TYPE_e,int)));

    delete recordingRetation;

    disconnect (recordingDrive,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

    disconnect (recordingDrive,
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotSelectionButtonClick(OPTION_STATE_TYPE_e,int)));

    delete recordingDrive;

    disconnect (recordingTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

    disconnect (recordingTextBox,
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotLoadInfoPage(int,INFO_MSG_TYPE_e)));

    delete recordingTextBox;
    delete recordingTextBoxParam;


    disconnect (recordingRetationCameraWise,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

    disconnect (recordingRetationCameraWise,
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotSelectionButtonClick(OPTION_STATE_TYPE_e,int)));

    delete recordingRetationCameraWise;


    disconnect (recordingSelectCamera,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

    disconnect (recordingSelectCamera,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));

    delete recordingSelectCamera;

    disconnect (backupRetation,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

    disconnect (backupRetation,
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotSelectionButtonClick(OPTION_STATE_TYPE_e,int)));

    delete backupRetation;


    disconnect (backupSelectCamera,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

    disconnect (backupSelectCamera,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));

    delete backupSelectCamera;


    disconnect (storageAlert,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

    disconnect (storageAlert,
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotSelectionButtonClick(OPTION_STATE_TYPE_e,int)));

    delete storageAlert;

    disconnect (storageAlertTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

    disconnect (storageAlertTextBox,
                SIGNAL(sigLoadInfopage(int,INFO_MSG_TYPE_e)),
                this,
                SLOT(slotLoadInfoPage(int,INFO_MSG_TYPE_e)));

    delete storageAlertTextBox;
    delete storageAlertTextBoxParam;

    if(recordRetSelCam != NULL)
    {
        disconnect (recordRetSelCam,
                    SIGNAL(sigDeleteObject(quint8)),
                    this,
                    SLOT(slotSubOjectDelete(quint8)));
        delete recordRetSelCam;
        recordRetSelCam = NULL;
    }

    if(backupRetSelCam != NULL)
    {
        disconnect (backupRetSelCam,
                    SIGNAL(sigDeleteObject(quint8)),
                    this,
                    SLOT(slotSubOjectDelete(quint8)));
        delete backupRetSelCam;
        backupRetSelCam = NULL;
    }
}

void StorageManagment::getCameraList ()
{
    QString tempStr;
    cameraList.clear();

    for(quint8 index = 0; index < devTableInfo->totalCams; index++)
    {
        tempStr = applController->GetCameraNameOfDevice(currDevName,index);
        cameraList.insert(index, QString("%1%2%3").arg(index + 1)
                          .arg(": ").arg (tempStr));
    }
}

void StorageManagment::createPayload(REQ_MSG_ID_e msgType)
{
    QString payloadString =
            payloadLib->createDevCnfgPayload(msgType,
                                             STORAGE_MANAGMENT_TABLE_INDEX,
                                             CNFG_FRM_INDEX,
                                             CNFG_FRM_INDEX,
                                             CNFG_FRM_INDEX,
                                             MAX_STR_MNG_FEILDS,
                                             MAX_STR_MNG_FEILDS);

    DevCommParam* param = new DevCommParam();
    param->msgType = msgType;
    param->payload = payloadString;

    processBar->loadProcessBar ();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void StorageManagment::defaultConfig ()
{
    createPayload (MSG_DEF_CFG);
}

void StorageManagment::getConfig ()
{
    createPayload (MSG_GET_CFG);
}

void StorageManagment::saveConfig ()
{
    if (IS_VALID_OBJ(storageTextBox) &&
            IS_VALID_OBJ(recordingTextBox) &&
            IS_VALID_OBJ(storageAlertTextBox))
    {
        if ((!storageTextBox->doneKeyValidation())
                || (!recordingTextBox->doneKeyValidation())
                || (!storageAlertTextBox->doneKeyValidation()))
        {
           return;
        }
    }

    setConfigFields();
    createPayload (MSG_SET_CFG);
}

void StorageManagment::setConfigFields ()
{
    quint8 temp;

    temp = ((storageOptions[ENBL_ALRT_STOP_REC]->getCurrentState () == ON_STATE) ?
                ENBL_ALRT_STOP_REC :
                (storageOptions[ENBL_OVER_FILE]->getCurrentState () == ON_STATE ?
                     ENBL_OVER_FILE : ENBL_CLN_OLDER_FILE));

    payloadLib->setCnfgArrayAtIndex(STR_HDD_ACT_MODE,temp);

    payloadLib->setCnfgArrayAtIndex(STR_HDD_CLN_SPEC,storageTextBox->getInputText());

    temp = recordingRetation->getCurrentState () == ON_STATE ? 1 : 0;

    payloadLib->setCnfgArrayAtIndex(STR_ENB_REG_CLN,temp);

    temp = recordingDrive->getCurrentState () == ON_STATE ? 0 : 1;

    payloadLib->setCnfgArrayAtIndex(STR_REC_ENBL_REC_RETN,temp);

    payloadLib->setCnfgArrayAtIndex(STR_REC_DRV_RETN_TIME,
                                    recordingTextBox->getInputText());

    temp = backupRetation->getCurrentState () == ON_STATE ? 1 : 0;

    payloadLib->setCnfgArrayAtIndex(STR_BCK_RET_ENB,temp);

    temp = storageAlert->getCurrentState () == ON_STATE ? 1 : 0;

    payloadLib->setCnfgArrayAtIndex(STR_REC_STOR_ENB,temp);

    payloadLib->setCnfgArrayAtIndex(STR_REC_STOR_MEM,
                                    storageAlertTextBox->getInputText());

    for(quint8 index = 0;index < MAX_CAMERAS; index++ )
    {
        payloadLib->setCnfgArrayAtIndex(STR_REC_RETN_TIME_CAM1 + index,
                                        recordingRetationValues.at (index));
        payloadLib->setCnfgArrayAtIndex(STR_BCK_RETN_TIME_CAM1 + index,
                                        backupRetationValues.at (index));
    }
}

void StorageManagment::processDeviceResponse (DevCommParam *param, QString deviceName)
{
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
                if(payloadLib->getcnfgTableIndex () == STORAGE_MANAGMENT_TABLE_INDEX)
                {
                    quint8 temp;
                    temp = payloadLib->getCnfgArrayAtIndex(STR_HDD_ACT_MODE).toUInt ();

                    storageOptions[ENBL_ALRT_STOP_REC]->changeState (temp == ENBL_ALRT_STOP_REC ? ON_STATE : OFF_STATE);
                    storageOptions[ENBL_OVER_FILE]->changeState (temp == ENBL_OVER_FILE ? ON_STATE : OFF_STATE);
                    storageOptions[ENBL_CLN_OLDER_FILE]->changeState (temp == ENBL_CLN_OLDER_FILE ? ON_STATE : OFF_STATE);

                    storageTextBox->setIsEnabled (temp == ENBL_CLN_OLDER_FILE ? true : false);
                    storageTextBox->setInputText (
                                payloadLib->getCnfgArrayAtIndex(STR_HDD_CLN_SPEC).toString ());

                    temp = payloadLib->getCnfgArrayAtIndex(STR_ENB_REG_CLN).toUInt ();

                    recordingRetation->changeState (temp == 1 ? ON_STATE : OFF_STATE);

                    temp = payloadLib->getCnfgArrayAtIndex(STR_REC_ENBL_REC_RETN).toUInt ();

                    recordingDrive->changeState (temp == 0 ? ON_STATE : OFF_STATE);

                    recordingRetationCameraWise->changeState (temp == 1 ? ON_STATE : OFF_STATE);

                    recordingTextBox->setInputText (
                                payloadLib->getCnfgArrayAtIndex(STR_REC_DRV_RETN_TIME).toString ());

                    temp = payloadLib->getCnfgArrayAtIndex(STR_BCK_RET_ENB).toUInt ();

                    backupRetation->changeState (temp == 1 ? ON_STATE : OFF_STATE);

                    temp = payloadLib->getCnfgArrayAtIndex(STR_REC_STOR_ENB).toUInt ();

                    storageAlert->changeState (temp == 1 ? ON_STATE : OFF_STATE);

                    storageAlertTextBox->setIsEnabled (temp == 1 ? true : false);

                    storageAlertTextBox->setInputText (
                                payloadLib->getCnfgArrayAtIndex(STR_REC_STOR_MEM).toString ());

                    recordingRetationValues.clear ();
                    backupRetationValues.clear ();

                    QString tempStr;

                    for(quint8 index = 0;index < MAX_CAMERAS; index++ )
                    {
                        tempStr = payloadLib->getCnfgArrayAtIndex
                                (STR_REC_RETN_TIME_CAM1 + index).toString ();
                        recordingRetationValues.insert (index,tempStr);

                        tempStr = payloadLib->getCnfgArrayAtIndex
                                (STR_BCK_RETN_TIME_CAM1 + index).toString ();
                        backupRetationValues.insert (index,tempStr);
                    }

                    if(recordingRetation->getCurrentState () == ON_STATE)
                    {
                        recordingDrive->setIsEnabled (true);
                        recordingRetationCameraWise->setIsEnabled (true);

                        if(recordingDrive->getCurrentState () == ON_STATE)
                        {
                            recordingRetationCameraWise->changeState (OFF_STATE);
                            recordingTextBox->setIsEnabled (true);
                            recordingSelectCamera->setIsEnabled (false);
                        }
                        else
                        {
                            recordingTextBox->setIsEnabled (false);
                            recordingSelectCamera->setIsEnabled (true);
                            recordingDrive->changeState (OFF_STATE);
                        }
                    }
                    else
                    {
                        recordingDrive->setIsEnabled (false);
                        recordingTextBox->setIsEnabled (false);
                        recordingSelectCamera->setIsEnabled (false);
                        recordingRetationCameraWise->setIsEnabled (false);
                    }

                    if(backupRetation->getCurrentState () == ON_STATE)
                    {
                        backupSelectCamera->setIsEnabled (true);
                    }
                    else
                    {
                        backupSelectCamera->setIsEnabled (false);
                    }
                    processBar->unloadProcessBar ();
                }
            }
                break;

            case MSG_SET_CFG:
            {
                processBar->unloadProcessBar ();
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
                getConfig ();
            }
                break;

            case MSG_DEF_CFG:
            {
                processBar->unloadProcessBar ();
                getConfig ();
            }
                break;

            default:
                break;
            }//inner switch

        } break;

        default:
            processBar->unloadProcessBar ();
            infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
            break;
        }
    }
}

void StorageManagment::slotSelectionButtonClick(OPTION_STATE_TYPE_e state,int index)
{
    switch(index)
    {
    case STR_ALRT_STOP_REC:
    {
        storageTextBox->setIsEnabled (false);
        storageOptions[ENBL_OVER_FILE]->changeState (OFF_STATE);
        storageOptions[ENBL_CLN_OLDER_FILE]->changeState (OFF_STATE);
    }
        break;

    case STR_OVERWRT_REC:
    {
        storageTextBox->setIsEnabled (false);
        storageOptions[ENBL_CLN_OLDER_FILE]->changeState (OFF_STATE);
        storageOptions[ENBL_ALRT_STOP_REC]->changeState (OFF_STATE);
    }
        break;

    case STR_CLN_REC:
    {
        storageTextBox->setIsEnabled (state);
        storageOptions[ENBL_OVER_FILE]->changeState (OFF_STATE);
        storageOptions[ENBL_ALRT_STOP_REC]->changeState (OFF_STATE);
    }
        break;

    case STR_REC_RTNT:
    {
        recordingDrive->setIsEnabled (state);
        recordingRetationCameraWise->setIsEnabled (state);

        if(state)
        {
            if(recordingDrive->getCurrentState () == ON_STATE)
            {
                recordingRetationCameraWise->changeState (OFF_STATE);
                recordingTextBox->setIsEnabled (true);
                recordingSelectCamera->setIsEnabled (false);
            }
            else
            {
                recordingTextBox->setIsEnabled (false);
                recordingSelectCamera->setIsEnabled (true);
                recordingDrive->changeState (OFF_STATE);
            }
        }
        else
        {
            recordingTextBox->setIsEnabled (false);
            recordingSelectCamera->setIsEnabled (false);
        }
    }
        break;

    case STR_REC_DRV_CTRL:
    {
        recordingRetationCameraWise->changeState (OFF_STATE);
        recordingTextBox->setIsEnabled (true);
        recordingSelectCamera->setIsEnabled (false);
    }
        break;

    case STR_REC_CAMWISE:
    {
        recordingTextBox->setIsEnabled (false);
        recordingSelectCamera->setIsEnabled (true);
        recordingDrive->changeState (OFF_STATE);
    }
        break;

    case STR_BCK_RTNT:
    {
        backupSelectCamera->setIsEnabled (state == ON_STATE ? true : false);
    }
        break;

    case STR_STOR_ALERT:
    {
        storageAlertTextBox->setIsEnabled (state);
    }
        break;

    default:
        break;

    }
}

void StorageManagment::slotButtonClick (int index)
{
    switch(index)
    {
    case STR_REC_CAM_SEL:
    {
        if(recordRetSelCam == NULL)
        {
            recordRetSelCam = new RecordRetention(cameraList,
                                                  recordingRetationValues,
                                                  parentWidget (),
                                                  storageManagmentStrings[STR_REC_RTET_MNG],
                                                  STR_REC_CAM_SEL);
            connect (recordRetSelCam,
                     SIGNAL(sigDeleteObject(quint8)),
                     this,
                     SLOT(slotSubOjectDelete(quint8)));
        }

    }
        break;

    case STR_BCK_CAM_SEL:
    {
        if(backupRetSelCam == NULL)
        {
            backupRetSelCam = new RecordRetention(cameraList,
                                                  backupRetationValues,
                                                  parentWidget (),
                                                  storageManagmentStrings[STR_REC_BCKUP_RTET_MNG],
                                                  STR_BCK_CAM_SEL);
            connect (backupRetSelCam,
                     SIGNAL(sigDeleteObject(quint8)),
                     this,
                     SLOT(slotSubOjectDelete(quint8)));
        }
    }
        break;

    default:
        break;
    }
}

void StorageManagment::slotSubOjectDelete (quint8 index)
{
    switch(index)
    {
    case STR_REC_CAM_SEL:
    {
        if(recordRetSelCam != NULL)
        {
            disconnect (recordRetSelCam,
                        SIGNAL(sigDeleteObject(quint8)),
                        this,
                        SLOT(slotSubOjectDelete(quint8)));
            delete recordRetSelCam;
            recordRetSelCam = NULL;
        }
    }
        break;

    case STR_BCK_CAM_SEL:
    {
        if(backupRetSelCam != NULL)
        {
            disconnect (backupRetSelCam,
                        SIGNAL(sigDeleteObject(quint8)),
                        this,
                        SLOT(slotSubOjectDelete(quint8)));
            delete backupRetSelCam;
            backupRetSelCam = NULL;
        }
    }
        break;

    default:
        break;
    }
    m_elementList[m_currentElement]->forceActiveFocus ();
}

void StorageManagment::slotLoadInfoPage(int index,INFO_MSG_TYPE_e msgType)
{
    switch (index)
    {
    case STR_CLN_REC_TXTBX:
    {
        if(msgType == INFO_MSG_ERROR)
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(STOR_MANAGE_ENT_PERCENT_DEFI_RANGE));
        }
    }
        break;

    case STR_REC_DRV_TXTBX:
    {
        if(msgType == INFO_MSG_ERROR)
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_VALUE_DEFI_RANGE));
        }
    }
        break;

    case STR_STOR_ALERT_TXTBX:
    {
        if(msgType == INFO_MSG_ERROR)
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(STOR_MANAGE_ENT_SPACE_VALUE_DEFI_RANGE));
        }
    }
        break;

    default:
        break;
    }
}
