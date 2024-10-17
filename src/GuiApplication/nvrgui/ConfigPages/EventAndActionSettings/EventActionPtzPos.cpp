#include "EventActionPtzPos.h"

#include <QPainter>
#include <QKeyEvent>

#define EVNT_PTZ_NOTIFY_WIDTH   SCALE_WIDTH(740)
#define EVNT_PTZ_NOTIFY_HEIGHT  SCALE_HEIGHT(230)
#define EVNT_PTZ_BG_WIDTH       SCALE_WIDTH(680)

typedef enum{

    EVNT_PTZ_HEADING,
    EVNT_PTZ_CAM_SPINBOX_LABEL,
    EVNT_PTZ_OK_LABEL,
    EVNT_PTZ_CANCEL_LABEL,

    MAX_EVNT_PTZ_NOTIFY_STRINGS
}EVNT_PTZ_NOTIFY_STRINGS_e;

static const QString eventActionPTZNotifyStrings[MAX_EVNT_PTZ_NOTIFY_STRINGS] = {
    "Preset Position",
    "Camera",
    "OK",
    "Cancel"
};

EventActionPtzPos::EventActionPtzPos(quint8 index,
                                     QString devName,
                                     quint32 &camNum,
                                     quint32 &prePosition,
                                     quint8 totalCam,
                                     QWidget *parent):
    KeyBoard(parent)
{
    this->setGeometry (0,0,parent->width (),parent->height ());
    m_maxCam = totalCam;
    m_index = index;

    for(quint8 index = 0; index < MAX_EVNT_PTZ_NOTIFY_CTRL; index++)
    {
        m_elementlist[index] = NULL;
    }

    currDevName = devName;
    camNumber =  &camNum ;
    presetPosition = &prePosition;

    payloadLib = new PayloadLib();

    applController = ApplController::getInstance ();

    backGround = new Rectangle(SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH) + SCALE_WIDTH(20) +
                               (SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) - EVNT_PTZ_NOTIFY_WIDTH)/2 ,
                               (SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)
                               +(SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT) - EVNT_PTZ_NOTIFY_HEIGHT)/2),
                               EVNT_PTZ_NOTIFY_WIDTH,
                               EVNT_PTZ_NOTIFY_HEIGHT,
                               0,
                               NORMAL_BKG_COLOR,
                               NORMAL_BKG_COLOR,
                               this);

    closeButton = new CloseButtton (backGround->x ()+backGround->width () - SCALE_WIDTH(20),
                                    backGround->y () + SCALE_HEIGHT(25),
                                    this,
                                    CLOSE_BTN_TYPE_1,
                                    EVNT_PTZ_CLS_CTRL);

    m_elementlist[EVNT_PTZ_CLS_CTRL] = closeButton;

    connect (closeButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    connect (closeButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    heading = new Heading(backGround->x () + backGround->width ()/2,
                          backGround->y () + SCALE_HEIGHT(25),
                          eventActionPTZNotifyStrings[EVNT_PTZ_HEADING],
                          this,
                          HEADING_TYPE_2);

    cameraList.insert (0,"None");

    cameraNameDropDownBox = new DropDown(backGround->x () + SCALE_WIDTH(25),
                                         backGround->y () + SCALE_HEIGHT(60),
                                         EVNT_PTZ_BG_WIDTH,
                                         BGTILE_HEIGHT,
                                         EVNT_PTZ_CAM_SPINBOX_CTRL,
                                         DROPDOWNBOX_SIZE_320,
                                         eventActionPTZNotifyStrings[EVNT_PTZ_CAM_SPINBOX_LABEL],
                                         cameraList,
                                         this,
                                         "",
                                         true,
                                         0,
                                         COMMON_LAYER,
                                         true,
                                         4);

    m_elementlist[EVNT_PTZ_CAM_SPINBOX_CTRL] = cameraNameDropDownBox;

    connect (cameraNameDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    connect (cameraNameDropDownBox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotSpinBoxValueChanged(QString,quint32)));

    currentCameraIndex = (camNum-1);
    fillCameraList ();

    positionList.clear ();
    positionList.insert (0,"None");

    presetPosDropDownBox = new DropDown(cameraNameDropDownBox->x () ,
                                        cameraNameDropDownBox->y () + BGTILE_HEIGHT,
                                        EVNT_PTZ_BG_WIDTH,
                                        BGTILE_HEIGHT,
                                        EVNT_PTZ_PRE_SPINBOX_CTRL,
                                        DROPDOWNBOX_SIZE_225,
                                        eventActionPTZNotifyStrings[EVNT_PTZ_HEADING],
                                        positionList,
                                        this,
                                        "",
                                        true,
                                        0,
                                        COMMON_LAYER,
                                        true,
                                        4);

    m_elementlist[EVNT_PTZ_PRE_SPINBOX_CTRL] = presetPosDropDownBox;

    connect (presetPosDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    okButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                              backGround->x () + backGround->width ()/2 - SCALE_WIDTH(60),
                              backGround->y () + backGround->height () - SCALE_HEIGHT(50),
                              eventActionPTZNotifyStrings[EVNT_PTZ_OK_LABEL],
                              this,
                              EVNT_PTZ_OK_CTRL);

    m_elementlist[EVNT_PTZ_OK_CTRL] = okButton;

    connect (okButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    connect (okButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    cancelButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                  backGround->x () + backGround->width ()/2 + SCALE_WIDTH(60),
                                  backGround->y () + backGround->height () - SCALE_HEIGHT(50),
                                  eventActionPTZNotifyStrings
                                  [EVNT_PTZ_CANCEL_LABEL],
                                  this,
                                  EVNT_PTZ_CANCEL_CTRL);

    m_elementlist[EVNT_PTZ_CANCEL_CTRL] = cancelButton;

    connect (cancelButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    connect (cancelButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    processBar = new ProcessBar(SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH),
                                SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT),
                                SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH),
                                SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT),
                                0,
                                this);

    currElement = EVNT_PTZ_CAM_SPINBOX_CTRL;
    m_elementlist[currElement]->forceActiveFocus ();

    getConfig();

    this->show ();
}

EventActionPtzPos::~EventActionPtzPos()
{
    delete payloadLib;
    delete processBar;

    delete backGround;
    delete heading;

    disconnect (closeButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    disconnect (closeButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete closeButton;

    disconnect (cameraNameDropDownBox,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotSpinBoxValueChanged(QString,quint32)));
    disconnect (cameraNameDropDownBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete cameraNameDropDownBox;

    disconnect (presetPosDropDownBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete presetPosDropDownBox;

    disconnect (okButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));

    disconnect (okButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

    delete okButton;

    disconnect (cancelButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));

    disconnect (cancelButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete cancelButton;
}

void EventActionPtzPos::paintEvent (QPaintEvent *)
{
    QPainter painter(this);
    QColor color;

    color.setAlpha (0);
    painter.setBrush (QBrush(color));
    painter.setPen (Qt::NoPen);

    painter.drawRoundedRect (QRect(0,
                                   0,
                                   SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH),
                                   SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT)),
                             SCALE_WIDTH(RECT_RADIUS), SCALE_HEIGHT(RECT_RADIUS));

    color.setAlpha (150);
    painter.setBrush (QBrush(color));
    painter.setPen (Qt::NoPen);

    painter.drawRoundedRect (QRect(SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH),
                                   SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) -SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT),
                                   SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH),
                                   SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)),
                             SCALE_WIDTH(RECT_RADIUS), SCALE_HEIGHT(RECT_RADIUS));
}

void EventActionPtzPos::processDeviceResponse(QMap<quint8, QString>  &presetList)
{
    processBar->unloadProcessBar ();
    presetPosDropDownBox->setNewList (presetList);
    presetPosDropDownBox->setCurrValue (presetList.value(*presetPosition));
}

void EventActionPtzPos::fillCameraList ()
{
    QString tempStr;
    cameraList.clear();

    for(quint8 index = 0; index < m_maxCam ; index++)
    {
        tempStr = applController->GetCameraNameOfDevice(currDevName,index);
        cameraList.insert(index, QString("%1%2%3").arg(index + 1)
                          .arg(" : ").arg (tempStr));
    }
    cameraNameDropDownBox->setNewList (cameraList,(currentCameraIndex));
}


void EventActionPtzPos::takeLeftKeyAction()
{
    do
    {
        currElement = (currElement - 1 + MAX_EVNT_PTZ_NOTIFY_CTRL) %
                MAX_EVNT_PTZ_NOTIFY_CTRL;
    }while(!m_elementlist[currElement]->getIsEnabled());

    m_elementlist[currElement]->forceActiveFocus();
}

void EventActionPtzPos::takeRightKeyAction()
{
    do
    {
        currElement = (currElement + 1) % MAX_EVNT_PTZ_NOTIFY_CTRL;
    }while(!m_elementlist[currElement]->getIsEnabled());

    m_elementlist[currElement]->forceActiveFocus();
}

void EventActionPtzPos::showEvent (QShowEvent *event)
{
    QWidget::showEvent (event);
    if(m_elementlist[currElement] != NULL)
    {
        m_elementlist[currElement]->forceActiveFocus();
    }
}

void EventActionPtzPos::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void EventActionPtzPos::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void EventActionPtzPos::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void EventActionPtzPos::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    currElement = EVNT_PTZ_CLS_CTRL;
    m_elementlist[currElement]->forceActiveFocus ();
}

void EventActionPtzPos::getConfig ()
{
    frmIndex = ((cameraNameDropDownBox->getIndexofCurrElement ()*MAX_PRESET_POS) + 1);
    createPayload(MSG_GET_CFG);
}

void EventActionPtzPos::createPayload(REQ_MSG_ID_e msgType )
{
    QString payloadString = payloadLib->createDevCnfgPayload(msgType,
                                                             PRESET_POSITION_TABLE_INDEX,
                                                             frmIndex,
                                                             frmIndex + (MAX_PRESET_POS-1),
                                                             1,
                                                             1,
                                                             1);

    DevCommParam* param = new DevCommParam();
    param->msgType = msgType;
    param->payload = payloadString;

    processBar->loadProcessBar ();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void EventActionPtzPos::slotUpdateCurrentElement(int index)
{
    currElement = index;
}

void EventActionPtzPos::slotButtonClick (int index)
{
    if(index == EVNT_PTZ_OK_CTRL)
    {
        *camNumber = (cameraNameDropDownBox->getIndexofCurrElement () + 1);
        *presetPosition = (presetPosDropDownBox->getIndexofCurrElement () );
    }
    emit sigDeleteObject(m_index);
}

void EventActionPtzPos::slotSpinBoxValueChanged (QString, quint32)
{
    currentCameraIndex = cameraNameDropDownBox->getIndexofCurrElement ();
    fillCameraList ();
    getConfig ();
}

void EventActionPtzPos::insertKeyPressed(QKeyEvent *event)
{
    event->accept();
}
