#include "USBManagment.h"
#include "UsbControl.h"
#include "ValidationMessage.h"

#define FAT_PARTITION 1

typedef enum {
    USB_MNG_FORMAT_CNTRL,
    USB_MNG_UNPLUG_CTRL,
    USB_MNG_STOP_CTRL,
    USB_BCK_FORMAT_CNTRL,
    USB_BCK_UNPLUG_CTRL,
    USB_BCK_STOP_CTRL,

    MAX_USB_MNG_CTRL
}USB_MNG_CTRL_e;

typedef enum {

    USB_DVC_TYPE,
    USB_TOTAL_SIZE,
    USB_FREE_SIZE,
    USB_DISK_STATUS,
    USB_BACKUP_STATUS,
    USB_FORMAT,
    USB_MNG_BCK_STR,
    USB_MNG_SCHD_STR,
    USB_MNG_FORMAT,
    USB_MNG_UNPLUG,
    USB_MNG_STOP_BACKUP,

    MAX_USB_MNG_STR
}USB_MNG_STR_e;


static const QString usbManagmentStrings[]={

    "Device Type",
    "Total Size (GB)",
    "Free Size (GB)",
    "Disk Status",
    "Backup Status",
    "Format as",
    "Manual Backup",
    "Scheduled Backup",
    "Format",
    "Unplug",
    "Stop Backup"
};


static const quint32 elementWidth[]={ READONLY_LARGE_WIDTH,
                                      READONLY_SMALL_WIDTH,
                                      READONLY_SMALL_WIDTH,
                                      READONLY_LARGE_WIDTH,
                                      READONLY_LARGE_WIDTH,
                                      READONLY_SMALL_WIDTH};

static const QString usbStatusStrings[]= { "No Disk",
                                           "Detected",
                                           "Formatting",
                                           "Fault",
                                           "Unplugging"};

static const QString usbBackupStatusStrings[] = { "Full",
                                                  "Backup is in Progress",
                                                  "Complete",
                                                  "Incomplete",
                                                  "Disable",
                                                  "Ready for Backup",
                                                  "Not Enough Space"};



USBManagment::USBManagment(QString devName, QWidget *parent)
    : ConfigPageControl(devName, parent,MAX_USB_MNG_CTRL,NULL,MAX_CNFG_BTN_TYPE),
      currentClickUsb(MAX_USB_TYPE)
{
    createDefaultComponents ();

    statusRepTimer = new QTimer(this);
    connect (statusRepTimer,
             SIGNAL(timeout()),
             this,
             SLOT(slotStatusRepTimerTimeout()));
    statusRepTimer->setInterval (5000);

    getUsbState();
}

void USBManagment::createDefaultComponents ()
{
    currentButtonClick = MAX_USB_MNG_CTRL;

    manualBackUpHeading = new ElementHeading ((SCALE_WIDTH(PAGE_RIGHT_PANEL_WIDTH) - 2*BGTILE_MEDIUM_SIZE_WIDTH)/2,
                                              (SCALE_HEIGHT(PAGE_RIGHT_PANEL_HEIGHT) - 9*BGTILE_HEIGHT)/2,
                                              BGTILE_MEDIUM_SIZE_WIDTH,
                                              BGTILE_HEIGHT,
                                              usbManagmentStrings[USB_MNG_BCK_STR],
                                              TOP_LAYER,
                                              this,
                                              false,
                                              SCALE_WIDTH(25), NORMAL_FONT_SIZE, true);

    schdBackUpHeading = new ElementHeading (manualBackUpHeading->x () + manualBackUpHeading ->width () + SCALE_WIDTH(5),
                                            manualBackUpHeading->y (),
                                            BGTILE_MEDIUM_SIZE_WIDTH,
                                            BGTILE_HEIGHT,
                                            usbManagmentStrings[USB_MNG_SCHD_STR],
                                            TOP_LAYER,
                                            this,
                                            false,
                                            SCALE_WIDTH(25), NORMAL_FONT_SIZE, true);

    for(quint8 index = 0 ; index < MAX_READONLY_ELEMENT;index++)
    {
        manualBackUpElements[index] = new ReadOnlyElement(manualBackUpHeading->x (),
                                                          manualBackUpHeading->y () +
                                                          manualBackUpHeading->height () +
                                                          index*BGTILE_HEIGHT,
                                                          BGTILE_MEDIUM_SIZE_WIDTH,
                                                          BGTILE_HEIGHT,
                                                          SCALE_WIDTH(elementWidth[index]),
                                                          READONLY_HEIGHT,
                                                          "-",
                                                          this,
                                                          MIDDLE_TABLE_LAYER,
                                                          -1,
                                                          SCALE_WIDTH(10),
                                                          usbManagmentStrings[USB_DVC_TYPE + index],
                                                          "",
                                                          SUFFIX_FONT_COLOR);

        schdBackUpElements[index] = new ReadOnlyElement(schdBackUpHeading->x (),
                                                        manualBackUpElements[index]->y (),
                                                        BGTILE_MEDIUM_SIZE_WIDTH,
                                                        BGTILE_HEIGHT,
                                                        SCALE_WIDTH(elementWidth[index]),
                                                        READONLY_HEIGHT,
                                                        "-",
                                                        this,
                                                        MIDDLE_TABLE_LAYER,
                                                        -1,
                                                        SCALE_WIDTH(10),
                                                        usbManagmentStrings[USB_DVC_TYPE + index],
                                                        "",
                                                        SUFFIX_FONT_COLOR);
    }


    manualBackUpControls[0] = new ControlButton (FORMAT_BUTTON_INDEX,
                                                 manualBackUpHeading->x (),
                                                 manualBackUpElements[MAX_READONLY_ELEMENT-1]->y ()
                                                 + BGTILE_HEIGHT,
                                                 BGTILE_MEDIUM_SIZE_WIDTH,
                                                 BGTILE_HEIGHT,
                                                 this,
                                                 DOWN_LAYER,
                                                 SCALE_WIDTH(40),
                                                 usbManagmentStrings[USB_MNG_FORMAT],
                                                 false,
                                                 USB_MNG_FORMAT_CNTRL);

    m_elementList[USB_MNG_FORMAT_CNTRL] = manualBackUpControls[0];

    connect(manualBackUpControls[0],
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClick(int)));

    connect (manualBackUpControls[0],
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    manualBackUpControls[1] = new ControlButton (UNPLUG_BUTTON_INDEX,
                                                 manualBackUpHeading->x () + SCALE_WIDTH(170),
                                                 manualBackUpElements[MAX_READONLY_ELEMENT-1]->y ()
                                                 + BGTILE_HEIGHT,
                                                 BGTILE_MEDIUM_SIZE_WIDTH,
                                                 BGTILE_HEIGHT,
                                                 this,
                                                 NO_LAYER,
                                                 SCALE_WIDTH(150),
                                                 usbManagmentStrings[USB_MNG_UNPLUG],
                                                 false,
                                                 USB_MNG_UNPLUG_CTRL);

    m_elementList[USB_MNG_UNPLUG_CTRL] = manualBackUpControls[1];

    connect(manualBackUpControls[1],
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClick(int)));

    connect (manualBackUpControls[1],
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    manualBackUpControls[2] = new ControlButton (STOP_BUTTON_INDEX,
                                                 manualBackUpHeading->x () + SCALE_WIDTH(300),
                                                 manualBackUpElements[MAX_READONLY_ELEMENT-1]->y ()
                                                 + BGTILE_HEIGHT,
                                                 BGTILE_MEDIUM_SIZE_WIDTH,
                                                 BGTILE_HEIGHT,
                                                 this,
                                                 NO_LAYER,
                                                 0,
                                                 usbManagmentStrings[USB_MNG_STOP_BACKUP],
                                                 false,
                                                 USB_MNG_STOP_CTRL);

    m_elementList[USB_MNG_STOP_CTRL] = manualBackUpControls[2];

    connect(manualBackUpControls[2],
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClick(int)));

    connect (manualBackUpControls[2],
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    schdBackUpControls[0] = new ControlButton (FORMAT_BUTTON_INDEX,
                                               schdBackUpHeading->x (),
                                               manualBackUpElements[MAX_READONLY_ELEMENT-1]->y ()
                                               + BGTILE_HEIGHT,
                                               BGTILE_MEDIUM_SIZE_WIDTH,
                                               BGTILE_HEIGHT,
                                               this,
                                               DOWN_LAYER,
                                               SCALE_WIDTH(40),
                                               usbManagmentStrings[USB_MNG_FORMAT],
                                               false,
                                               USB_BCK_FORMAT_CNTRL);

    m_elementList[USB_BCK_FORMAT_CNTRL] = schdBackUpControls[0];

    connect(schdBackUpControls[0],
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClick(int)));

    connect (schdBackUpControls[0],
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    schdBackUpControls[1] = new ControlButton (UNPLUG_BUTTON_INDEX,
                                               schdBackUpHeading->x () + SCALE_WIDTH(170),
                                               manualBackUpElements[MAX_READONLY_ELEMENT-1]->y ()
                                               + BGTILE_HEIGHT,
                                               BGTILE_MEDIUM_SIZE_WIDTH,
                                               BGTILE_HEIGHT,
                                               this,
                                               NO_LAYER,
                                               SCALE_WIDTH(150),
                                               usbManagmentStrings[USB_MNG_UNPLUG],
                                               false,
                                               USB_BCK_UNPLUG_CTRL);

    m_elementList[USB_BCK_UNPLUG_CTRL] = schdBackUpControls[1];

    connect(schdBackUpControls[1],
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClick(int)));

    connect (schdBackUpControls[1],
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));




    schdBackUpControls[2] = new ControlButton (STOP_BUTTON_INDEX,
                                               schdBackUpHeading->x () + SCALE_WIDTH(300),
                                               manualBackUpElements[MAX_READONLY_ELEMENT-1]->y ()
                                               + BGTILE_HEIGHT,
                                               BGTILE_MEDIUM_SIZE_WIDTH,
                                               BGTILE_HEIGHT,
                                               this,
                                               NO_LAYER,
                                               0,
                                               usbManagmentStrings[USB_MNG_STOP_BACKUP],
                                               false,
                                               USB_BCK_STOP_CTRL);

    m_elementList[USB_BCK_STOP_CTRL] = schdBackUpControls[2];

    connect(schdBackUpControls[2],
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotButtonClick(int)));

    connect (schdBackUpControls[2],
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    m_elementList[m_currentElement]->forceActiveFocus();

}

USBManagment::~USBManagment()
{
    statusRepTimer->stop ();
    delete statusRepTimer;

    delete manualBackUpHeading;
    delete schdBackUpHeading;

    for(quint8 index = 0 ; index < MAX_READONLY_ELEMENT; index++)
    {
        delete manualBackUpElements[index];
        delete schdBackUpElements[index];
    }

    for(quint8 index = 0; index < MAX_USB_CNTRL; index++)
    {
        disconnect(manualBackUpControls[index],
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonClick(int)));

        disconnect (manualBackUpControls[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete manualBackUpControls[index];

        disconnect(schdBackUpControls[index],
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotButtonClick(int)));

        disconnect (schdBackUpControls[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete schdBackUpControls[index];
    }
}

void USBManagment::sendCommand(SET_COMMAND_e cmdType,int totalfeilds)
{
    QString payloadString = payloadLib->createDevCmdPayload(totalfeilds);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = cmdType;
    param->payload = payloadString;

    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void USBManagment::getUsbState ()
{
    if(!statusRepTimer->isActive ())
    {
        statusRepTimer->start ();
    }

    sendCommand(USB_DISK_STS);
}

void USBManagment::formatUsb (int index)
{
    payloadLib->setCnfgArrayAtIndex (0,index);
    payloadLib->setCnfgArrayAtIndex (1,FAT_PARTITION);

    sendCommand(USBFORMAT,2);
}

void USBManagment::unplugUsb (int index)
{
    payloadLib->setCnfgArrayAtIndex (0,index);

    sendCommand(USBUNPLUG,1);
}

void USBManagment::stopBackup (int index)
{
    payloadLib->setCnfgArrayAtIndex (0,index);

    sendCommand(STP_BCKUP,1);
}

void USBManagment::handleInfoPageMessage(int index)
{
    if(index == INFO_OK_BTN)
    {
        switch(currentButtonClick)
        {
        case USB_MNG_FORMAT_CNTRL:
            formatUsb(USB_TYPE_MANUAL);
            break;

        case USB_MNG_UNPLUG_CTRL:
            currentClickUsb = USB_TYPE_MANUAL;
            unplugUsb(USB_TYPE_MANUAL);
            break;

        case USB_MNG_STOP_CTRL:
            stopBackup(USB_TYPE_MANUAL);
            break;

        case USB_BCK_FORMAT_CNTRL:
            formatUsb(USB_TYPE_SCHEDULED);
            break;

        case USB_BCK_UNPLUG_CTRL:
            currentClickUsb = USB_TYPE_SCHEDULED;
            unplugUsb(USB_TYPE_SCHEDULED);
            break;

        case USB_BCK_STOP_CTRL:
            stopBackup(USB_TYPE_SCHEDULED);
            break;

        default:
            break;
        }
    }
    else
    {
        m_elementList[m_currentElement]->forceActiveFocus ();
    }
}

void USBManagment::processDeviceResponse (DevCommParam *param, QString deviceName)
{
    if (deviceName == currDevName)
    {
        switch(param->deviceStatus)
        {
        case CMD_SUCCESS:
            switch(param->msgType)
            {
            case MSG_SET_CMD:
                payloadLib->parseDevCmdReply(true, param->payload);
                switch(param->cmdType)
                {
                case USB_DISK_STS:
                    quint8 tempIndex;

                    manualBackUpElements[USB_DVC_TYPE]->changeValue
                            (payloadLib->getCnfgArrayAtIndex (USB_DVC_TYPE).toString ());

                    manualBackUpElements[USB_TOTAL_SIZE]->changeValue
                            (payloadLib->getCnfgArrayAtIndex (USB_TOTAL_SIZE).toString ());

                    manualBackUpElements[USB_FREE_SIZE]->changeValue
                            (payloadLib->getCnfgArrayAtIndex (USB_FREE_SIZE).toString ());

                    tempIndex = payloadLib->getCnfgArrayAtIndex (USB_DISK_STATUS).toUInt ();
                    manualBackUpElements[USB_FORMAT]->changeValue ("FAT");
                    if((tempIndex == 1) || (tempIndex == 3))
                    {
                        manualBackUpControls[0]->setIsEnabled (true);
                        manualBackUpControls[1]->setIsEnabled (true);
                    }
                    else
                    {
                        manualBackUpControls[0]->setIsEnabled (false);
                        manualBackUpControls[1]->setIsEnabled (false);
                    }

                    manualBackUpElements[USB_DISK_STATUS]->changeValue
                            (usbStatusStrings[tempIndex]);

                    tempIndex = payloadLib->getCnfgArrayAtIndex (USB_BACKUP_STATUS + 1).toUInt ();

                    manualBackUpControls[2]->setIsEnabled ((tempIndex == 1) ? true : false);

                    manualBackUpElements[USB_BACKUP_STATUS]->changeValue
                            (usbBackupStatusStrings[tempIndex]);

                    schdBackUpElements[USB_DVC_TYPE]->changeValue
                            (payloadLib->getCnfgArrayAtIndex (USB_DVC_TYPE + 6).toString ());

                    schdBackUpElements[USB_TOTAL_SIZE]->changeValue
                            (payloadLib->getCnfgArrayAtIndex (USB_TOTAL_SIZE + 6).toString ());

                    schdBackUpElements[USB_FREE_SIZE]->changeValue
                            (payloadLib->getCnfgArrayAtIndex (USB_FREE_SIZE + 6).toString ());

                    tempIndex = payloadLib->getCnfgArrayAtIndex (USB_DISK_STATUS + 6).toUInt ();
                    schdBackUpElements[USB_FORMAT]->changeValue ("FAT");
                    if((tempIndex == 1) || (tempIndex == 3))
                    {
                        schdBackUpControls[0]->setIsEnabled (true);
                        schdBackUpControls[1]->setIsEnabled (true);
                        //                        schdBackUpFormat->setIsEnabled (true);
                    }
                    else
                    {
                        schdBackUpControls[0]->setIsEnabled (false);
                        schdBackUpControls[1]->setIsEnabled (false);
                        //                        schdBackUpFormat->setIsEnabled (false);
                    }

                    schdBackUpElements[USB_DISK_STATUS]->changeValue
                            (usbStatusStrings[tempIndex]);

                    tempIndex = payloadLib->getCnfgArrayAtIndex (USB_BACKUP_STATUS + 1 + 6).toUInt ();

                    schdBackUpControls[2]->setIsEnabled ((tempIndex == 1) ? true : false);

                    schdBackUpElements[USB_BACKUP_STATUS]->changeValue
                            (usbBackupStatusStrings[tempIndex]);
                    break;

                case USBFORMAT:
                    getUsbState ();
                    break;

                case USBUNPLUG:
                    UsbControl::updateShowUsbFlag ((currentClickUsb + 1),false);
                    infoPage->loadInfoPage (ValidationMessage::getValidationMessage(USB_MANAGE_USB_EJECT_SUCCESS));
                    currentClickUsb = MAX_USB_TYPE;
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
            currentClickUsb = MAX_USB_TYPE;
            infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
            break;
        }

        if(param->cmdType != USB_DISK_STS)
        {
            currentButtonClick = MAX_USB_MNG_CTRL;
        }
    }
}

void USBManagment::slotButtonClick (int index)
{
    currentButtonClick = index;
    switch(currentButtonClick)
    {
    case USB_BCK_FORMAT_CNTRL:
    case USB_MNG_FORMAT_CNTRL:
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(USB_MANAGE_FORMAT_DEV), true);
        break;

    case USB_BCK_UNPLUG_CTRL:
    case USB_MNG_UNPLUG_CTRL:
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(USB_MANAGE_UNPLUG_DEV), true);
        break;

    case USB_BCK_STOP_CTRL:
    case USB_MNG_STOP_CTRL:
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(USB_MANAGE_STOP_BACKUP),true);
        break;

    default:
        break;
    }

}

void USBManagment::slotStatusRepTimerTimeout()
{
    getUsbState ();
}
