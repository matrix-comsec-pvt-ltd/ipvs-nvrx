#include "EventActionDeviceAlarm.h"

#include <QPainter>
#include <QKeyEvent>

#define EVNT_DEV_ALM_NOTIFY_WIDTH   SCALE_WIDTH(536)
#define EVNT_DEV_ALM_NOTIFY_HEIGHT  SCALE_HEIGHT(230)

typedef enum{

    EVNT_DEV_ALM_HEADING,
    EVNT_DEV_ALM_ELE_HEADING,
    EVNT_DEV_ALM_OK_LABEL,
    EVNT_DEV_ALM_CANCEL_LABEL,

    MAX_EVNT_DEV_ALM_NOTIFY_STRINGS
}EVNT_DEV_ALM_NOTIFY_STRINGS_e;

static const QString eventActionDeviceAlarmNotifyStrings[MAX_EVNT_DEV_ALM_NOTIFY_STRINGS] = {
    "Device Alarm",
    "Select Alarm Output",
    "OK",
    "Cancel"
};

EventActionDeviceAlarm::EventActionDeviceAlarm(quint8 index,bool *deviceAlrm,
                                               DEV_TABLE_INFO_t *devTableInfo,QWidget *parent) :
    KeyBoard(parent)
{
    this->setGeometry (0,0,parent->width (),parent->height ());

    m_devTableInfo = devTableInfo;
    m_index = index;
    currElement = EVNT_DEV_ALM_CLS_CTRL;

    for(quint8 index = 0; index < MAX_EVNT_DEV_ALM_NOTIFY_CTRL; index++)
    {
        m_elementlist[index] = NULL;
    }

    devAlm = deviceAlrm;
    isAllAlarmActivated = 0 ;

    backGround = new Rectangle(SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH) + SCALE_WIDTH(20) +
                               (SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) - EVNT_DEV_ALM_NOTIFY_WIDTH)/2 ,
                               (SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)
                               +(SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT) - EVNT_DEV_ALM_NOTIFY_HEIGHT)/2),
                               EVNT_DEV_ALM_NOTIFY_WIDTH,
                               EVNT_DEV_ALM_NOTIFY_HEIGHT,
                               0,
                               NORMAL_BKG_COLOR,
                               NORMAL_BKG_COLOR,
                               this);

    closeButton = new CloseButtton (backGround->x ()+ backGround->width () - SCALE_WIDTH(30),
                                    backGround->y () + SCALE_HEIGHT(25),
                                    this,
                                    CLOSE_BTN_TYPE_1,
                                    EVNT_DEV_ALM_CLS_CTRL);

    m_elementlist[EVNT_DEV_ALM_CLS_CTRL] = closeButton;

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
                          eventActionDeviceAlarmNotifyStrings[EVNT_DEV_ALM_HEADING],
                          this,
                          HEADING_TYPE_2);

    elementHeading = new ElementHeading ( backGround->x () + SCALE_WIDTH(25),
                                          backGround->y () + SCALE_HEIGHT(50),
                                          BGTILE_MEDIUM_SIZE_WIDTH,
                                          BGTILE_HEIGHT,
                                          eventActionDeviceAlarmNotifyStrings
                                          [EVNT_DEV_ALM_ELE_HEADING],
                                          UP_LAYER,
                                          this,
                                          false,
                                          SCALE_WIDTH(20), NORMAL_FONT_SIZE, true);

    bgTile = new BgTile(elementHeading->x (),
                        elementHeading->y () + BGTILE_HEIGHT,
                        BGTILE_MEDIUM_SIZE_WIDTH,
                        BGTILE_HEIGHT,
                        MIDDLE_TABLE_LAYER,
                        this);

    quint16 labelMargin[]={152,210,255,297,340};
    int labelWidth;
    if(m_devTableInfo->alarms > 0)
    {
        labelWidth = QFontMetrics(TextLabel::getFont(NORMAL_FONT_FAMILY, NORMAL_FONT_SIZE)).width("All");
        alarmLabel[0] = new TextLabel(elementHeading->x () + SCALE_WIDTH(152),
                                      bgTile->y () + SCALE_HEIGHT(10),
                                      NORMAL_FONT_SIZE,
                                      "All",
                                      this,
                                      NORMAL_FONT_COLOR,
                                      NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                      0, 0, labelWidth);

        for(quint8 index = 1; index < (m_devTableInfo->alarms+1); index++)
        {
            alarmLabel[index] = new TextLabel(elementHeading->x () + SCALE_WIDTH(labelMargin[index]),
                                              bgTile->y () + SCALE_HEIGHT(10),
                                              NORMAL_FONT_SIZE,
                                              QString("%1").arg(index),
                                              this,
                                              NORMAL_FONT_COLOR,
                                              NORMAL_FONT_FAMILY);
        }


        alarmCheckBox[0] = new OptionSelectButton(bgTile->x (),
                                                  bgTile->y () + BGTILE_HEIGHT,
                                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                                  BGTILE_HEIGHT,
                                                  CHECK_BUTTON_INDEX,
                                                  this,
                                                  BOTTOM_TABLE_LAYER,
                                                  "","",
                                                  SCALE_WIDTH(140),
                                                  EVNT_DEV_ALM_ALL_CHCKBOX);

        m_elementlist[EVNT_DEV_ALM_ALL_CHCKBOX] = alarmCheckBox[0];
        currElement = EVNT_DEV_ALM_ALL_CHCKBOX;

        connect (alarmCheckBox[0],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

        connect (alarmCheckBox[0],
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotCheckboxClicked(OPTION_STATE_TYPE_e,int)));


        alarmCheckBox[1] = new OptionSelectButton(bgTile->x () + SCALE_WIDTH(200),
                                                  bgTile->y () + BGTILE_HEIGHT,
                                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                                  BGTILE_HEIGHT,
                                                  CHECK_BUTTON_INDEX,
                                                  this,
                                                  NO_LAYER,
                                                  "","",
                                                  SCALE_WIDTH(140),
                                                  EVNT_DEV_ALM_ALARM1_CHCKBOX);

        m_elementlist[EVNT_DEV_ALM_ALARM1_CHCKBOX] = alarmCheckBox[1];

        connect (alarmCheckBox[1],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

        connect (alarmCheckBox[1],
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotCheckboxClicked(OPTION_STATE_TYPE_e,int)));

        for(quint8 index = 2; index < (m_devTableInfo->alarms + 1); index++)
        {
            alarmCheckBox[index] = new OptionSelectButton(alarmCheckBox[index-1]->x () +
                    alarmCheckBox[index-1]->width () + SCALE_WIDTH(20),
                    alarmCheckBox[index-1]->y (),
                    BGTILE_MEDIUM_SIZE_WIDTH,
                    BGTILE_HEIGHT,
                    CHECK_BUTTON_INDEX,
                    this,
                    NO_LAYER,
                    "","",
                    SCALE_WIDTH(140),
                    EVNT_DEV_ALM_ALL_CHCKBOX + index);

            m_elementlist[EVNT_DEV_ALM_ALL_CHCKBOX + index] = alarmCheckBox[index];

            connect (alarmCheckBox[index],
                     SIGNAL(sigUpdateCurrentElement(int)),
                     this,
                     SLOT(slotUpdateCurrentElement(int)));

            connect (alarmCheckBox[index],
                     SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                     this,
                     SLOT(slotCheckboxClicked(OPTION_STATE_TYPE_e,int)));
        }
    }

    okButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                              backGround->x () + backGround->width ()/2 - SCALE_WIDTH(60),
                              backGround->y () + backGround->height () - SCALE_HEIGHT(25),
                              eventActionDeviceAlarmNotifyStrings
                              [EVNT_DEV_ALM_OK_LABEL],
                              this,
                              EVNT_DEV_ALM_OK_CTRL);

    m_elementlist[EVNT_DEV_ALM_OK_CTRL] = okButton;

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
                                  backGround->y () + backGround->height () - SCALE_HEIGHT(25),
                                  eventActionDeviceAlarmNotifyStrings
                                  [EVNT_DEV_ALM_CANCEL_LABEL],
                                  this,
                                  EVNT_DEV_ALM_CANCEL_CTRL);

    m_elementlist[EVNT_DEV_ALM_CANCEL_CTRL] = cancelButton;

    connect (cancelButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    connect (cancelButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    assignValues();
    m_elementlist[currElement]->forceActiveFocus ();

    this->show ();
}

EventActionDeviceAlarm::~EventActionDeviceAlarm()
{
    disconnect (cancelButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    disconnect (cancelButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete cancelButton;

    disconnect (okButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    disconnect (okButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete okButton;

    if(m_devTableInfo->alarms > 0)
    {
        for(quint8 index = 0; index < (m_devTableInfo->alarms + 1); index++)
        {
            disconnect (alarmCheckBox[index],
                        SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                        this,
                        SLOT(slotCheckboxClicked(OPTION_STATE_TYPE_e,int)));
            disconnect (alarmCheckBox[index],
                        SIGNAL(sigUpdateCurrentElement(int)),
                        this,
                        SLOT(slotUpdateCurrentElement(int)));
            delete alarmCheckBox[index];
            delete alarmLabel[index];
        }
    }

    disconnect (closeButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    disconnect (closeButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete closeButton;
    delete bgTile;
    delete heading;
    delete elementHeading;
    delete backGround;
}

void EventActionDeviceAlarm::paintEvent (QPaintEvent *)
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

void EventActionDeviceAlarm::assignValues ()
{
    quint8 temp = 0;
    if(m_devTableInfo->alarms > 0)
    {
        for(quint8 index = 1; index < (m_devTableInfo->alarms + 1); index ++)
        {
            alarmCheckBox[index]->changeState (devAlm[(index-1)] == true ?
                        ON_STATE : OFF_STATE);
            if(devAlm[(index-1)] == true )
            {
                temp++;
            }
        }

        alarmCheckBox[0]->changeState ((temp == m_devTableInfo->alarms) ? ON_STATE : OFF_STATE);
    }
}

void EventActionDeviceAlarm::takeLeftKeyAction()
{
    bool status = true;
    do
    {
        if(currElement == 0)
        {
            currElement = (MAX_EVNT_DEV_ALM_NOTIFY_CTRL);
        }
        if(currElement)
        {
            currElement = (currElement - 1);
        }
        else
        {
              status = false;
              break;
        }

    }while((m_elementlist[currElement] == NULL)
           ||(!m_elementlist[currElement]->getIsEnabled()));

    if(status == true)
    {
        m_elementlist[currElement]->forceActiveFocus();
    }
}

void EventActionDeviceAlarm::takeRightKeyAction()
{
    bool status = true;
    do
    {
        if(currElement == (MAX_EVNT_DEV_ALM_NOTIFY_CTRL - 1))
        {
            currElement = -1;
        }
        if(currElement != (MAX_EVNT_DEV_ALM_NOTIFY_CTRL - 1))
        {
            currElement = (currElement + 1);
        }
        else
        {
            status = false;
            break;
        }
    }while((m_elementlist[currElement] == NULL)
           ||(!m_elementlist[currElement]->getIsEnabled()));

    if(status == true)
    {
        m_elementlist[currElement]->forceActiveFocus();
    }
}

void EventActionDeviceAlarm::showEvent (QShowEvent *event)
{
    QWidget::showEvent (event);
    if(m_elementlist[currElement] != NULL)
    {
        m_elementlist[currElement]->forceActiveFocus();
    }
}

void EventActionDeviceAlarm::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void EventActionDeviceAlarm::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void EventActionDeviceAlarm::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void EventActionDeviceAlarm::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    currElement = EVNT_DEV_ALM_CLS_CTRL;
    m_elementlist[currElement]->forceActiveFocus ();
}

void EventActionDeviceAlarm::slotUpdateCurrentElement(int index)
{
    currElement = index;
}

void EventActionDeviceAlarm::slotButtonClick (int index)
{
    if(index == EVNT_DEV_ALM_OK_CTRL)
    {
        for(quint8 index = 1; index < (m_devTableInfo->alarms + 1); index ++)
        {
            devAlm[(index-1)] = (alarmCheckBox[index]->getCurrentState () == ON_STATE) ?
                        true : false;
        }
    }
    emit sigDeleteObject(m_index);
}

void EventActionDeviceAlarm::slotCheckboxClicked (OPTION_STATE_TYPE_e state, int index)
{
    if(index == EVNT_DEV_ALM_ALL_CHCKBOX)
    {
        for(quint8 index = 1; index < (m_devTableInfo->alarms + 1); index ++)
        {
            alarmCheckBox[index]->changeState (state);
        }
    }
    else
    {
        quint8 temp = 0;
        for(quint8 index = 1; index < (m_devTableInfo->alarms + 1); index ++)
        {
            if(alarmCheckBox[index]->getCurrentState () == ON_STATE)
            {
                temp++;
            }
        }
        if(m_devTableInfo->alarms > 0)
        {
            alarmCheckBox[0]->changeState ((temp == m_devTableInfo->alarms) ? ON_STATE : OFF_STATE);
        }
    }
}

void EventActionDeviceAlarm::insertKeyPressed(QKeyEvent *event)
{
    event->accept();
}
