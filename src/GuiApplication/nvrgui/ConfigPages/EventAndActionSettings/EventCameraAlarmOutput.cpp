#include "EventCameraAlarmOutput.h"

#include <QPainter>
#include <QKeyEvent>

#define EVENT_CAM_ALRM_WIDTH      (SCALE_WIDTH(536) + SCALE_WIDTH(160))
#define EVENT_CAM_ALRM_HEIGHT     SCALE_HEIGHT(330)

static const QString eventCameraAlarm[] = { "Camera Alarm Output",
                                            "Camera",
                                            "Select Camera Output",
                                            "All",
                                            "OK",
                                            "Cancel"
                                          };


EventCameraAlarmOutput::EventCameraAlarmOutput(quint8 indx,
                                               QMap<quint8, QString> cameraList,
                                               bool* camAlarmStatus,
                                               quint8 &camIndex,
                                               QWidget *parent) :
    KeyBoard(parent)
{
    this->setGeometry (0,0,parent->width (),parent->height ());
    indexInPage = indx;
    camSelected = &camIndex;
    cameraAlarmStatus = camAlarmStatus;

    backGround = new Rectangle(SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH) + SCALE_WIDTH(20) +
                               (SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) - EVENT_CAM_ALRM_WIDTH)/2 ,
                               (SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)
                               +(SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT) - EVENT_CAM_ALRM_HEIGHT)/2),
                               EVENT_CAM_ALRM_WIDTH,
                               EVENT_CAM_ALRM_HEIGHT,
                               EVENT_CAM_ALAM_CLS,
                               NORMAL_BKG_COLOR,
                               NORMAL_BKG_COLOR,
                               this);

    closeButton = new CloseButtton (backGround->x ()+backGround->width () - SCALE_WIDTH(30),
                                    backGround->y () + SCALE_HEIGHT(25),
                                    this,
                                    CLOSE_BTN_TYPE_1,
                                    0);

    m_elementlist[EVENT_CAM_ALAM_CLS] = closeButton;

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
                          eventCameraAlarm[0],
                          this,
                          HEADING_TYPE_2);

    cameraNameDropDownBox = new DropDown(backGround->x () + SCALE_WIDTH(20),
                                         backGround->y () + SCALE_HEIGHT(80),
                                         BGTILE_MEDIUM_SIZE_WIDTH + SCALE_WIDTH(160),
                                         BGTILE_HEIGHT,
                                         1,
                                         DROPDOWNBOX_SIZE_320,
                                         eventCameraAlarm[1],
                                         cameraList,
                                         this,
                                         "",
                                         true,0,
                                         COMMON_LAYER,
                                         true,
                                         4);

    cameraNameDropDownBox->setIndexofCurrElement (*camSelected -1);

    m_elementlist[EVENT_CAM_SPINBOX] = cameraNameDropDownBox ;

    connect (cameraNameDropDownBox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotSpinboxValueChange(QString,quint32)));

    connect (cameraNameDropDownBox,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));


    elementHeading = new ElementHeading(cameraNameDropDownBox->x (),
                                        cameraNameDropDownBox->y () +
                                        cameraNameDropDownBox->height (),
                                        cameraNameDropDownBox->width (),
                                        BGTILE_HEIGHT,
                                        eventCameraAlarm[2],
                                        UP_LAYER,
                                        this,
                                        false,
                                        SCALE_WIDTH(25), NORMAL_FONT_SIZE, true);

    numberTile = new BgTile(elementHeading->x (),
                            elementHeading->y () +
                            elementHeading->height (),
                            elementHeading->width (),
                            BGTILE_HEIGHT,
                            MIDDLE_TABLE_LAYER,
                            this);

    quint16 labelMargin[] = { 265, 315, 357, 400};

    camAlaramCheckBoxNumber[0] = new TextLabel(elementHeading->x () + SCALE_WIDTH(labelMargin[0]),
                                               numberTile->y () + SCALE_HEIGHT(7),
                                               NORMAL_FONT_SIZE,
                                               eventCameraAlarm[3],
                                               this);

    for(quint8 index = 1; index < (MAX_CAM_ALARM + 1); index++)
    {
        camAlaramCheckBoxNumber[index] = new TextLabel(elementHeading->x () + SCALE_WIDTH(labelMargin[index]),
                                                       numberTile->y () + SCALE_HEIGHT(7),
                                                       NORMAL_FONT_SIZE,
                                                       QString("%1").arg (index),
                                                       this);
    }

    camAlaramCheckBox[0] = new OptionSelectButton(numberTile->x (),
                                                  numberTile->y () +
                                                  numberTile->height (),
                                                  numberTile->width (),
                                                  BGTILE_HEIGHT,
                                                  CHECK_BUTTON_INDEX,
                                                  this,
                                                  BOTTOM_TABLE_LAYER,
                                                  "",
                                                  "",
                                                  SCALE_WIDTH(250),
                                                  EVENT_CAM_ALAM_ALL_CHECKBOX);

    m_elementlist[EVENT_CAM_ALAM_ALL_CHECKBOX] = camAlaramCheckBox[0];

    connect (camAlaramCheckBox[0],
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotOptionsClicked(OPTION_STATE_TYPE_e,int)));

    connect (camAlaramCheckBox[0],
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    camAlaramCheckBox[1] = new OptionSelectButton(camAlaramCheckBox[0]->x () + SCALE_WIDTH(307),
                                                  camAlaramCheckBox[0]->y (),
                                                  numberTile->width (),
                                                  BGTILE_HEIGHT,
                                                  CHECK_BUTTON_INDEX,
                                                  this,
                                                  NO_LAYER,
                                                  "",
                                                  "",
                                                  SCALE_WIDTH(180),
                                                  EVENT_CAM_ALAM1_CHECKBOX);

    m_elementlist[EVENT_CAM_ALAM1_CHECKBOX] = camAlaramCheckBox[1];

    connect ( camAlaramCheckBox[1],
              SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
              this,
              SLOT(slotOptionsClicked(OPTION_STATE_TYPE_e,int)));

    connect (camAlaramCheckBox[1],
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    for(quint8 index = 2; index < (MAX_CAM_ALARM + 1); index++)
    {
        camAlaramCheckBox[index] = new OptionSelectButton(camAlaramCheckBox[index-1]->x () +
                                                          camAlaramCheckBox[index-1]->width () + SCALE_WIDTH(20),
                                                          camAlaramCheckBox[index-1]->y (),
                                                          numberTile->width (),
                                                          BGTILE_HEIGHT,
                                                          CHECK_BUTTON_INDEX,
                                                          this,
                                                          NO_LAYER,
                                                          "",
                                                          "",
                                                          SCALE_WIDTH(180),
                                                          EVENT_CAM_ALAM_ALL_CHECKBOX + index);

        m_elementlist[ EVENT_CAM_ALAM_ALL_CHECKBOX + index] = camAlaramCheckBox[index];

        connect ( camAlaramCheckBox[index],
                  SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                  this,
                  SLOT(slotOptionsClicked(OPTION_STATE_TYPE_e,int)));

        connect (camAlaramCheckBox[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));

    }

    okButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                              backGround->x () + backGround->width ()/2 - SCALE_WIDTH(60),
                              backGround->y () + backGround->height () - SCALE_HEIGHT(35),
                              eventCameraAlarm[4],
                              this,
                              EVENT_CAM_OK_BTN);

    m_elementlist[EVENT_CAM_OK_BTN] = okButton;

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
                                  backGround->y () + backGround->height () - SCALE_HEIGHT(35),
                                  eventCameraAlarm[5],
                                  this,
                                  EVENT_CAM_CANCEL_BTN);

    m_elementlist[EVENT_CAM_CANCEL_BTN] = cancelButton;

    connect (cancelButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    connect (cancelButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    assignValues();

    currElement = EVENT_CAM_SPINBOX;
    m_elementlist[currElement]->forceActiveFocus ();

    this->show ();
}

EventCameraAlarmOutput::~EventCameraAlarmOutput()
{
    delete backGround;

    disconnect (closeButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    disconnect (closeButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete closeButton;

    delete heading;

    disconnect (cameraNameDropDownBox,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotSpinboxValueChange(QString,quint32)));
    disconnect (cameraNameDropDownBox,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete cameraNameDropDownBox ;

    delete elementHeading;
    delete numberTile;

    for(quint8 index = 0; index < (MAX_CAM_ALARM + 1); index++)
    {
        disconnect (camAlaramCheckBox[index],
                    SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                    this,
                    SLOT(slotOptionsClicked(OPTION_STATE_TYPE_e,int)));
        disconnect (camAlaramCheckBox[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete camAlaramCheckBox[index];
        delete camAlaramCheckBoxNumber[index];
    }

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

void EventCameraAlarmOutput::paintEvent (QPaintEvent *)
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


void EventCameraAlarmOutput::takeLeftKeyAction()
{
    do
    {
        currElement = (currElement - 1 + MAX_EVENT_CAM_ALAM_CTRL) %
                MAX_EVENT_CAM_ALAM_CTRL;
    }while(!m_elementlist[currElement]->getIsEnabled());

    m_elementlist[currElement]->forceActiveFocus();
}

void EventCameraAlarmOutput::takeRightKeyAction()
{
    do
    {
        currElement = (currElement + 1) % MAX_EVENT_CAM_ALAM_CTRL;
    }while(!m_elementlist[currElement]->getIsEnabled());

    m_elementlist[currElement]->forceActiveFocus();
}

void EventCameraAlarmOutput::showEvent (QShowEvent *event)
{
    QWidget::showEvent (event);
    if(m_elementlist[currElement] != NULL)
    {
        m_elementlist[currElement]->forceActiveFocus();
    }
}

void EventCameraAlarmOutput::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void EventCameraAlarmOutput::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}


void EventCameraAlarmOutput::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void EventCameraAlarmOutput::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    currElement = EVENT_CAM_ALAM_CLS;
    m_elementlist[currElement]->forceActiveFocus ();
}

void EventCameraAlarmOutput::assignValues ()
{
    quint8 temp = 0;
    for(quint8 index = 1; index < (MAX_CAM_ALARM + 1); index ++)
    {
        camAlaramCheckBox[index]->changeState (cameraAlarmStatus[(index-1)] == true ?
                                                   ON_STATE : OFF_STATE);
        if(cameraAlarmStatus[(index-1)] == true )
        {
            temp++;
        }
    }
    camAlaramCheckBox[0]->changeState ((temp == MAX_CAM_ALARM) ? ON_STATE : OFF_STATE);
}

void EventCameraAlarmOutput::resetValues ()
{
    for(quint8 index = 1; index < (MAX_CAM_ALARM + 1); index ++)
    {
        camAlaramCheckBox[index]->changeState (OFF_STATE);
    }
}

void EventCameraAlarmOutput::slotUpdateCurrentElement(int index)
{
    currElement = index;
}

void EventCameraAlarmOutput::slotButtonClick (int index)
{
    if( index == EVENT_CAM_OK_BTN)
    {
        *camSelected = cameraNameDropDownBox->getIndexofCurrElement () + 1;
        for(quint8 index = 1; index < (MAX_CAM_ALARM + 1); index ++)
        {
            cameraAlarmStatus[(index-1)] = (camAlaramCheckBox[index]->getCurrentState () == ON_STATE) ?
                        true : false;
        }
    }

    emit sigDeleteObject(indexInPage);
}

void EventCameraAlarmOutput::slotOptionsClicked (OPTION_STATE_TYPE_e state,int tIndexInPage)
{
    switch(tIndexInPage)
    {
    case EVENT_CAM_ALAM_ALL_CHECKBOX:
    {
        for(quint8 index = 1; index < (MAX_CAM_ALARM + 1); index ++)
        {
            camAlaramCheckBox[index]->changeState (state);
        }
    }
        break;

    case EVENT_CAM_ALAM1_CHECKBOX:
    case EVENT_CAM_ALAM2_CHECKBOX:
    case EVENT_CAM_ALAM3_CHECKBOX:
    {
        quint8 temp = 0;
        for(quint8 index = 1; index < (MAX_CAM_ALARM + 1); index ++)
        {
            if(camAlaramCheckBox[index]->getCurrentState () == ON_STATE)
            {
                temp++;
            }
        }
        camAlaramCheckBox[0]->changeState ((temp == MAX_CAM_ALARM) ? ON_STATE : OFF_STATE);
    }
        break;

    default:
        break;
    }
}

void EventCameraAlarmOutput::slotSpinboxValueChange (QString, quint32)
{
    for(quint8 index = 0; index < (MAX_CAM_ALARM + 1); index ++)
    {
        camAlaramCheckBox[index]->changeState (OFF_STATE);
    }

    if(*camSelected == (cameraNameDropDownBox->getIndexofCurrElement () + 1))
    {
        assignValues();
    }
    else
    {
        resetValues ();
    }
}

void EventCameraAlarmOutput::insertKeyPressed(QKeyEvent *event)
{
    event->accept();
}
