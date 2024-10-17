#include "MxAnalogPresetMenu.h"
#include "ApplicationMode.h"
#include "ValidationMessage.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>

#define ANALOG_PRESET_CTRL_WIDTH      SCALE_WIDTH(500)
#define ANALOG_PRESET_CTRL_HEIGHT     SCALE_HEIGHT(195)

#define PTZ_CTRL_HEADING    "Camera Settings Menu"



typedef enum
{
    LIVEVIEW_ANALOG_PRESET_POS,

    MAX_LIVEVIEW_ANALOG_PRESET_ELEMETS
}LIVEVIEW_ANALOG_PRESET_ELELIST_e;



static const QString analogPresetMenuStrings[] =
{
    "Enter Preset Position",
    "Go"
};

//static const QString analogPresetElementSuffixStrings[MAX_LIVEVIEW_ANALOG_PRESET_ELEMETS]={
//    "(0-999)"


MxAnalogPresetMenu::MxAnalogPresetMenu(qint32 startx,
                                       qint32 starty,
                                       QString deviceName,
                                       quint8 cameraNum,
                                       QWidget *parent) :
    BackGround(parent->x() +( parent->width() - ANALOG_PRESET_CTRL_WIDTH)/2 ,
               parent->y() +( parent->height() - ANALOG_PRESET_CTRL_HEIGHT)/2 ,
               ANALOG_PRESET_CTRL_WIDTH,
               ANALOG_PRESET_CTRL_HEIGHT,
               BACKGROUND_TYPE_4,
               MAX_TOOLBAR_BUTTON,parent,0,0,PTZ_CTRL_HEADING,0,true),
    m_startX(startx), m_startY(starty), currentDeviceName(deviceName), camNo(cameraNum)
{
    currentButtonClick = 0;
    m_currentElement = 0;
    m_presetValue = 0;
    INIT_OBJ(imageTopBgTile);
    INIT_OBJ(m_analogPresetMenu);

    createDefaultComponent();

    this->show();
}

MxAnalogPresetMenu::~MxAnalogPresetMenu()
{
    if(IS_VALID_OBJ(m_mainCloseButton))
    {
        disconnect(m_mainCloseButton,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonClicked(int)));
        disconnect(m_mainCloseButton,
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));

        disconnect (m_goButton,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        disconnect (m_goButton,
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotButtonClicked(int)));
        delete m_goButton;

    }
    if(IS_VALID_OBJ(m_analogPresetPosTextBox))
    {
        disconnect (m_analogPresetPosTextBox,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        disconnect (m_analogPresetPosTextBox,
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotButtonClicked(int)));
        DELETE_OBJ(m_analogPresetPosTextBox);
    }
    if(IS_VALID_OBJ(payloadLib))
    {
        DELETE_OBJ(payloadLib);
    }
    if(IS_VALID_OBJ(m_analogPresetParam))
    {
        DELETE_OBJ(m_analogPresetParam);
    }
    if(IS_VALID_OBJ(m_devResponseLabel))
    {
        DELETE_OBJ(m_devResponseLabel);
    }
}

void MxAnalogPresetMenu::createDefaultComponent ()
{
    applController = ApplController::getInstance ();
    INIT_OBJ(payloadLib);
    INIT_OBJ(m_analogPresetParam);
    INIT_OBJ(m_analogPresetPosTextBox);
    payloadLib = new PayloadLib();

    m_analogPresetParam = new TextboxParam();
    m_analogPresetParam->labelStr = analogPresetMenuStrings[LIVEVIEW_ANALOG_PRESET_POS];
    //    m_analogPresetParam->suffixStr = analogPresetElementSuffixStrings[ANALOG_PRESET_POSITION_TXTBOX];
    m_analogPresetParam->isNumEntry=true;
    m_analogPresetParam->minNumValue=0;
    m_analogPresetParam->maxNumValue=999;
    m_analogPresetParam->maxChar=3;
    m_analogPresetPosTextBox = new TextBox((ANALOG_PRESET_CTRL_WIDTH - BGTILE_SMALL_SIZE_WIDTH )/2 ,
                                           (ANALOG_PRESET_CTRL_HEIGHT+ SCALE_HEIGHT(55) - 2*BGTILE_HEIGHT)/2,
                                           BGTILE_SMALL_SIZE_WIDTH,
                                           BGTILE_HEIGHT,
                                           ANALOG_PRESET_POSITION_TXTBOX,
                                           TEXTBOX_MEDIAM,
                                           this,
                                           m_analogPresetParam);

    m_elementList[ANALOG_PRESET_POSITION_TXTBOX] = m_analogPresetPosTextBox;


    connect (m_analogPresetPosTextBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));


    m_goButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                m_analogPresetPosTextBox->x () +this->width()/2 - SCALE_WIDTH(60),
                                m_analogPresetPosTextBox->y () +  m_analogPresetPosTextBox->height() + SCALE_HEIGHT(40),
                                analogPresetMenuStrings[1],
                                this,
                                ANALOG_PRESET_GO_BUTTON);

    m_elementList[ANALOG_PRESET_GO_BUTTON] = m_goButton;

    connect (m_goButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    connect (m_goButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClicked(int)));


    if(IS_VALID_OBJ(m_mainCloseButton))
    {
        connect(m_mainCloseButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClicked(int)));
        connect(m_mainCloseButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

        m_elementList[ANALOG_PRESET_CLS_BTN] = m_mainCloseButton;
    }
    INIT_OBJ(m_devResponseLabel);
    m_devResponseLabel = new TextLabel((this->width()/2 - SCALE_WIDTH(10))+ SCALE_WIDTH(5),
                                       SCALE_HEIGHT(50),
                                       SCALE_FONT(SMALL_SUFFIX_FONT_SIZE),
                                       "",
                                       this,
                                       POPUP_STATUS_LINE_COLOR,
                                       NORMAL_FONT_FAMILY,
                                       ALIGN_CENTRE_X_START_Y);

    if(IS_VALID_OBJ(m_devResponseLabel))
    {
        m_devResponseLabel->setVisible(false);
    }
}

void MxAnalogPresetMenu::slotButtonClicked (int index)
{
    switch(index)
    {
    case ANALOG_PRESET_CLS_BTN:
        //emit signal for delete object
        emit sigObjectDelete();
        break;

    case ANALOG_PRESET_GO_BUTTON:
        // send command for preset position.
        if(m_analogPresetPosTextBox->getInputText() == "")
        {
            m_devResponseLabel->changeText(ValidationMessage::getValidationMessage(PTZ_PRESET_POS_EMPTY));
            m_devResponseLabel->setVisible(true);
            break;
        }
        sendSetPresetGoCmd();
        break;

    default:
        break;
    }
}

void MxAnalogPresetMenu::slotUpdateCurrentElement (int index)
{
    m_currentElement = index;
}

void MxAnalogPresetMenu::sendSetPresetGoCmd()
{
    payloadLib->setCnfgArrayAtIndex (0,camNo);
    payloadLib->setCnfgArrayAtIndex (1,m_analogPresetPosTextBox->getInputText());
    createCmdPayload(SET_PRESET,2);
}

void MxAnalogPresetMenu::createCmdPayload (SET_COMMAND_e cmdType, quint8 totalFields)
{
    QString payloadString = payloadLib->createDevCmdPayload(totalFields);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = cmdType;
    param->payload = payloadString;
    applController->processActivity(currentDeviceName, DEVICE_COMM, param);
}

void MxAnalogPresetMenu::processDeviceResponse (DevCommParam *param, QString deviceName)
{
    if(deviceName == currentDeviceName)
    {
        switch(param->cmdType)
        {
        case SET_PRESET:
        {
            if(param->deviceStatus == CMD_SUCCESS)
            {
                m_devResponseLabel->setVisible(false);

                emit sigObjectDelete();
            }
            else
            {
                if(IS_VALID_OBJ(m_devResponseLabel))
                {
                    m_devResponseLabel->changeText(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                    m_devResponseLabel->setVisible(true);
                }
            }
        }
            break;

        default:
            break;
        }
    }
}
