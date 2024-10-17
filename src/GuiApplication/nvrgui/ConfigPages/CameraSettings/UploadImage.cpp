#include "UploadImage.h"
#include "ValidationMessage.h"


#define EMAIL_SERVER "Email Server"

#define UPLOAD_IMAGE_BGWIDTH (BGTILE_MEDIUM_SIZE_WIDTH + SCALE_WIDTH(190))

static const QStringList uploadServers = QStringList()
        << "Email Server"
        <<  "FTP Server 1"
         <<  "FTP Server 2"
          <<  "Network Drive 1"
           <<  "Network Drive 2";

static const QStringList imageRateList = QStringList()
        << "1" << "2" << "3" << "4" << "5";

typedef enum{
    UPLD_RESOLUTION = 0,
    UPLD_UPLOAD_LOCATION,
    UPLD_EMAIL_ADDRESS,
    UPLD_SUBJECT,
    UPLD_TEXT,
    UPLD_IMAGES_PER_MIN,
    MAX_UPLD_FEILDS
}UPLD_FIELDS_e;

typedef enum {
    UPLD_CAM_SPINBOX_LABEL,
    UPLD_UPLOAD_SPINBOX_LABEL,
    UPLD_EMAIL_TEXTBOX_LABEL,
    UPLD_SUBJECT_TEXTBOX_LABEL,
    UPLD_MSGBOX_LABEL,
    UPLD_IMAGE_RATE_SPINBOX_LABEL,
    UPLD_IMAGE_RATE_SPINBOX_SUFFIX,
    MAX_UPLD_STRINGS
}UPLD_STRINGS_e;


static const QString uploadImageString[MAX_UPLD_STRINGS]={
    "Camera",
    "Upload To",
    "Email Address",
    "Subject",
    "Message",
    "Maximum Images",
    "Per Minute"
};

UploadImage::UploadImage(QString deviceName,QWidget *parent, DEV_TABLE_INFO_t *devTabInfo)
    : ConfigPageControl(deviceName, parent,MAX_UPLOAD_IMAGE_CTRL,devTabInfo),
          currentCameraIndex(1)
{
    createDefaultComponent();
    UploadImage::getConfig();
}

void UploadImage::fillCameraList()
{
    QString tempStr;

    cameraNameList.clear();

    for(quint8 index = 0; index < devTableInfo->totalCams; index++)
    {
        tempStr = applController->GetCameraNameOfDevice (currDevName,index);

        if(((index + 1) < 10) && (devTableInfo->totalCams > 10))
        {
            cameraNameList.insert(index, QString(" %1%2%3").arg(index + 1)
                                  .arg(" : ").arg (tempStr));
        }
        else
        {
            cameraNameList.insert(index, QString("%1%2%3").arg(index + 1)
                                  .arg(" : ").arg (tempStr));
        }
    }
    cameraNameDropDownBox->setNewList (cameraNameList,(currentCameraIndex -1));
}

void UploadImage::createDefaultComponent()
{
    cameraNameList.clear ();
    emailNoteLabel = NULL;

    cameraNameDropDownBox = new DropDown((SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) - UPLOAD_IMAGE_BGWIDTH)/2 - SCALE_WIDTH(20),
                                         SCALE_HEIGHT(140),
                                         UPLOAD_IMAGE_BGWIDTH,
                                         BGTILE_HEIGHT,
                                         UPLD_CAMERA_NAME_SPINBOX_CTRL,
                                         DROPDOWNBOX_SIZE_320,
                                         uploadImageString[UPLD_CAM_SPINBOX_LABEL],
                                         cameraNameList,
                                         this);

    m_elementList[UPLD_CAMERA_NAME_SPINBOX_CTRL] = cameraNameDropDownBox;

    connect (cameraNameDropDownBox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotSpinboxValueChange(QString,quint32)));
    connect (cameraNameDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));


    QMap<quint8, QString> uploadServersList;
    uploadServersList.clear ();

    for(quint8 index = 0; index < uploadServers.length (); index++)
    {
        uploadServersList.insert (index,uploadServers.at(index));
    }

    uploadListDropDownBox = new DropDown(cameraNameDropDownBox->x (),
                                         cameraNameDropDownBox->y () + BGTILE_HEIGHT,
                                         UPLOAD_IMAGE_BGWIDTH,
                                         BGTILE_HEIGHT,
                                         UPLD_UPLOAD_SPINBOX_CTRL,
                                         DROPDOWNBOX_SIZE_225,
                                         uploadImageString[UPLD_UPLOAD_SPINBOX_LABEL],
                                         uploadServersList,
                                         this);

    m_elementList[UPLD_UPLOAD_SPINBOX_CTRL] = uploadListDropDownBox;

    connect (uploadListDropDownBox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotSpinboxValueChange(QString,quint32)));

    connect (uploadListDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));


    emailTextBoxParam = new TextboxParam();
    emailTextBoxParam->maxChar = 100;
    emailTextBoxParam->isEmailAddrType = true;
    emailTextBoxParam->labelStr = uploadImageString[UPLD_EMAIL_TEXTBOX_LABEL];

    emailTextBox = new TextBox(cameraNameDropDownBox->x (),
                               uploadListDropDownBox->y () + BGTILE_HEIGHT,
                               UPLOAD_IMAGE_BGWIDTH,
                               BGTILE_HEIGHT,
                               UPLD_EMAIL_TEXTBOX,
                               TEXTBOX_ULTRALARGE,
                               this,
                               emailTextBoxParam,
                               COMMON_LAYER,
                               false);

    m_elementList[UPLD_EMAIL_TEXTBOX] = emailTextBox;

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
    subjectTextBoxParam->labelStr = uploadImageString[UPLD_SUBJECT_TEXTBOX_LABEL];

    subjectTextBox = new TextBox(cameraNameDropDownBox->x (),
                                 emailTextBox->y () + BGTILE_HEIGHT,
                                 UPLOAD_IMAGE_BGWIDTH,
                                 BGTILE_HEIGHT,
                                 UPLD_SUBJECT_TEXTBOX,
                                 TEXTBOX_ULTRALARGE,
                                 this,
                                 subjectTextBoxParam,
                                 COMMON_LAYER,false);

    m_elementList[UPLD_SUBJECT_TEXTBOX] = subjectTextBox;

    connect (subjectTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));


    msgBox =  new MessageBox(cameraNameDropDownBox->x (),
                             subjectTextBox->y () + BGTILE_HEIGHT,
                             UPLOAD_IMAGE_BGWIDTH,
                             UPLD_MSGBOX,
                             this,
                             uploadImageString[UPLD_MSGBOX_LABEL],
                             COMMON_LAYER,
                             true,
                             0,
                             300,
                             QRegExp(""),
                             false,
                             false,
                             false);

    m_elementList[UPLD_MSGBOX] = msgBox;

    connect (msgBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));


    QMap<quint8, QString> imageRateMapList;
    imageRateMapList.clear ();

    for(quint8 index = 0; index < imageRateList.length (); index++)
    {
        imageRateMapList.insert (index,imageRateList.at(index));
    }

    imageRateDropDownBox = new DropDown(cameraNameDropDownBox->x (),
                                        msgBox->y () + 2*BGTILE_HEIGHT,
                                        UPLOAD_IMAGE_BGWIDTH,
                                        BGTILE_HEIGHT,
                                        UPLD_IMAGE_RATE_SPINBOX,
                                        DROPDOWNBOX_SIZE_90,
                                        uploadImageString[UPLD_IMAGE_RATE_SPINBOX_LABEL],
                                        imageRateMapList,
                                        this,
                                        uploadImageString[UPLD_IMAGE_RATE_SPINBOX_SUFFIX],
                                        true,
                                        0,
                                        COMMON_LAYER,
                                        true,
                                        4);

    m_elementList[UPLD_IMAGE_RATE_SPINBOX] = imageRateDropDownBox;

    connect (imageRateDropDownBox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotSpinboxValueChange(QString,quint32)));

    connect (imageRateDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    QString emailNote = QString("Note : Multiple Email IDs can be added using (;) or (,)");

    emailNoteLabel = new TextLabel(imageRateDropDownBox->x(),
                                   imageRateDropDownBox->y() + (BGTILE_HEIGHT),
                                   NORMAL_FONT_SIZE,
                                   emailNote,
                                   this);

   emailNoteLabel->setVisible(false);

}

UploadImage::~UploadImage()
{

    disconnect (imageRateDropDownBox,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotSpinboxValueChange(QString,quint32)));
    disconnect (imageRateDropDownBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete imageRateDropDownBox;

    disconnect (msgBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete msgBox;

    disconnect (subjectTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete subjectTextBox;
    delete subjectTextBoxParam;

    disconnect (emailTextBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete emailTextBox;
    delete emailTextBoxParam;

    disconnect (uploadListDropDownBox,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotSpinboxValueChange(QString,quint32)));
    disconnect (uploadListDropDownBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete uploadListDropDownBox;

    disconnect (cameraNameDropDownBox,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotSpinboxValueChange(QString,quint32)));
    disconnect (cameraNameDropDownBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete cameraNameDropDownBox;
    cameraNameList.clear ();

    if(IS_VALID_OBJ(emailNoteLabel))
    {
        DELETE_OBJ(emailNoteLabel);
    }

}


void UploadImage::getConfig ()
{
    fillCameraList();
    createPayload (MSG_GET_CFG);
}

void UploadImage::defaultConfig ()
{
    createPayload (MSG_DEF_CFG);
}

void UploadImage::saveConfig ()
{
    if ( uploadListDropDownBox->getIndexofCurrElement() == 0)
    {
        if ( (!emailTextBox->doneKeyValidation()))
        {
            return;
        }
    }

    if(saveConfigFeilds())
        createPayload (MSG_SET_CFG);
}

bool UploadImage::saveConfigFeilds ()
{
    if(EMAIL_SERVER == uploadListDropDownBox->getCurrValue ())
    {
        if(emailTextBox->getInputText () == "")
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_EMAIL_ADD));
            cameraNameDropDownBox->setIndexofCurrElement (currentCameraIndex-1);
            return false;
        }
        else if (subjectTextBox->getInputText () == "")
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_SUBJECT));
            cameraNameDropDownBox->setIndexofCurrElement (currentCameraIndex-1);
            return false;
        }
        else if (msgBox->getInputText () == "")
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(ENT_MESS));
            cameraNameDropDownBox->setIndexofCurrElement (currentCameraIndex-1);
            return false;
        }
    }

    payloadLib->setCnfgArrayAtIndex (UPLD_RESOLUTION,resolution);
    payloadLib->setCnfgArrayAtIndex (UPLD_UPLOAD_LOCATION,
                                     uploadListDropDownBox->getIndexofCurrElement ());
    payloadLib->setCnfgArrayAtIndex (UPLD_EMAIL_ADDRESS,
                                     emailTextBox->getInputText () );
    payloadLib->setCnfgArrayAtIndex (UPLD_SUBJECT,
                                     subjectTextBox->getInputText ());
    payloadLib->setCnfgArrayAtIndex (UPLD_MSGBOX,
                                     msgBox->getInputText ());
    payloadLib->setCnfgArrayAtIndex (UPLD_IMAGES_PER_MIN,
                                     imageRateDropDownBox->getCurrValue());

    return true;
}

void UploadImage::createPayload(REQ_MSG_ID_e msgType )
{
    QString payloadString = payloadLib->createDevCnfgPayload(msgType,
                                                             UPLOAD_IMAGE_TABLE_INDEX,
                                                             currentCameraIndex,
                                                             currentCameraIndex,
                                                             CNFG_FRM_INDEX,
                                                             MAX_UPLD_FEILDS,
                                                             MAX_UPLD_FEILDS);
    DevCommParam* param = new DevCommParam();
    param->msgType = msgType;
    param->payload = payloadString;

    processBar->loadProcessBar ();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void UploadImage::processDeviceResponse(DevCommParam *param, QString deviceName)
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
                m_configResponse.clear();

                payloadLib->parsePayload (param->msgType, param->payload);
                if(payloadLib->getcnfgTableIndex () == UPLOAD_IMAGE_TABLE_INDEX)
                {
                    resolution = (payloadLib->getCnfgArrayAtIndex (UPLD_RESOLUTION).toString ());
                    m_configResponse[UPLD_RESOLUTION]=resolution;

                    uploadListDropDownBox->setIndexofCurrElement (
                                payloadLib->getCnfgArrayAtIndex (UPLD_UPLOAD_LOCATION).toString () != "" ?
                                payloadLib->getCnfgArrayAtIndex (UPLD_UPLOAD_LOCATION).toUInt () : 0);

                    m_configResponse[UPLD_UPLOAD_LOCATION]= payloadLib->getCnfgArrayAtIndex (UPLD_UPLOAD_LOCATION);

                    emailTextBox->setInputText (payloadLib->getCnfgArrayAtIndex
                                                (UPLD_EMAIL_ADDRESS).toString ());
                    m_configResponse[UPLD_EMAIL_ADDRESS]=payloadLib->getCnfgArrayAtIndex (UPLD_EMAIL_ADDRESS);

                    subjectTextBox->setInputText (payloadLib->getCnfgArrayAtIndex
                                                  (UPLD_SUBJECT).toString ());
                    m_configResponse[UPLD_SUBJECT]=payloadLib->getCnfgArrayAtIndex (UPLD_SUBJECT);

                    msgBox->setInputText (payloadLib->getCnfgArrayAtIndex
                                          (UPLD_TEXT).toString ());
                    m_configResponse[UPLD_TEXT]=payloadLib->getCnfgArrayAtIndex (UPLD_TEXT);


                    imageRateDropDownBox->setCurrValue(payloadLib->getCnfgArrayAtIndex(UPLD_IMAGES_PER_MIN).toString());
                    m_configResponse[UPLD_IMAGES_PER_MIN]=payloadLib->getCnfgArrayAtIndex (UPLD_IMAGES_PER_MIN);


                    slotSpinboxValueChange (uploadListDropDownBox->getCurrValue (),
                                            UPLD_UPLOAD_SPINBOX_CTRL);
                }

            }
                break;

            case MSG_SET_CFG:
            {
                isUnloadProcessbar = false;
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
                currentCameraIndex = (cameraNameDropDownBox->getIndexofCurrElement () + 1);
                getConfig ();
            }
                break;

            case MSG_DEF_CFG:
            {
                isUnloadProcessbar = false;
                currentCameraIndex = (cameraNameDropDownBox->getIndexofCurrElement () + 1);
                getConfig ();
            }
                break;

            default:
                break;

            }
        }
            break;

        default:
            isUnloadProcessbar = false;
            processBar->unloadProcessBar ();
            infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
			cameraNameDropDownBox->setIndexofCurrElement (currentCameraIndex);
            break;
        }
    }

    if(isUnloadProcessbar)
    {
        processBar->unloadProcessBar ();
    }
}

bool UploadImage::isUserChangeConfig()
{
    bool isChange = false;

    do
    {
        if(m_configResponse.isEmpty())
        {
            break;
        }

        if((IS_VALID_OBJ(uploadListDropDownBox)) && (m_configResponse[UPLD_UPLOAD_LOCATION] != uploadListDropDownBox->getIndexofCurrElement()) )
        {
            isChange = true;
            break;
        }

        if((IS_VALID_OBJ(emailTextBox)) && (m_configResponse[UPLD_EMAIL_ADDRESS] != emailTextBox->getInputText() ))
        {
            isChange = true;
            break;
        }

        if((IS_VALID_OBJ(subjectTextBox)) && (m_configResponse[UPLD_SUBJECT] != subjectTextBox->getInputText()) )
        {
            isChange = true;
            break;
        }

        if((IS_VALID_OBJ(msgBox)) && (m_configResponse[UPLD_TEXT] !=  msgBox->getInputText() ))
        {
            isChange = true;
            break;
        }

        if((IS_VALID_OBJ(imageRateDropDownBox)) && (m_configResponse[UPLD_IMAGES_PER_MIN] !=  imageRateDropDownBox->getCurrValue() ))
        {
            isChange = true;
            break;
        }

    }while(0);

    return isChange;
}


void UploadImage::handleInfoPageMessage(int index)
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
            currentCameraIndex = (cameraNameDropDownBox->getIndexofCurrElement () + 1);
            getConfig();
        }
    }
}

void UploadImage::slotSpinboxValueChange (QString str , quint32 index)
{
    switch(index)
    {
    case  UPLD_UPLOAD_SPINBOX_CTRL :
    {
        if(str == EMAIL_SERVER )
        {
            msgBox->setIsEnabled (true);
            emailTextBox->setIsEnabled (true);
            subjectTextBox->setIsEnabled (true);
            emailNoteLabel->setVisible(true);
        }
        else
        {
            if(msgBox->isEnabled ())
                msgBox->setIsEnabled (false);
            if(emailTextBox->isEnabled ())
                emailTextBox->setIsEnabled (false);
            if(subjectTextBox->isEnabled ())
                subjectTextBox->setIsEnabled (false);
            emailNoteLabel->setVisible(false);
        }
    }
        break;

    case UPLD_CAMERA_NAME_SPINBOX_CTRL:
    {
        if(isUserChangeConfig())
        {
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(SAVE_CHANGES),true,false,"",CONFORMATION_BTN_YES,CONFORMATION_BTN_NO);
        }
        else
        {
            currentCameraIndex = (cameraNameDropDownBox->getIndexofCurrElement () + 1);
            getConfig ();
        }
    }
        break;

    default:
        break;
    }
}
void UploadImage::slotTextBoxInfoPage(int index ,INFO_MSG_TYPE_e msgType)
{
    if(index == UPLD_EMAIL_TEXTBOX)
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
