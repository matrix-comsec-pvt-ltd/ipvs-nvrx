#include "PTZSchd.h"
#include "ValidationMessage.h"

#include <QPainter>
#include <QKeyEvent>

#define PTZ_PAGE_WIDTH   SCALE_WIDTH(1260)
#define PTZ_PAGE_HEIGHT  SCALE_HEIGHT(530)
#define MAX_ELE_IN_ROW   8

typedef enum{

    PTZ_SCHD_ENTIRE_DAY,
    PTZ_SCHD_ENT_DAY_PRE,
    PTZ_SCHD_TIME1_STRT_TIME,
    PTZ_SCHD_TIME1_ED_TIME,
    PTZ_SCHD_TIME1_PRE,
    PTZ_SCHD_TIME2_STRT_TIME,
    PTZ_SCHD_TIME2_ED_TIME,
    PTZ_SCHD_TIME2_PRE,

    MAX_PTZ_SCHD_ROW_ELE
}PTZ_SCHD_ROW_ELE_e;


typedef enum{

    PTZ_SCHD_HEADING,
    PTZ_SCHD_ELE_HEADING1,
    PTZ_SCHD_ELE_HEADING2,
    PTZ_SCHD_ELE_HEADING3,
    PTZ_SCHD_ELE_HEADING4,
    PTZ_SCHD_PRE_TOUR_HEADING,
    PTZ_SCHD_STRT_TIME_HEADING,
    PTZ_SCHD_END_TIME_HEADING,
    PTZ_SCHD_PRE_TOUR_HEADING2,
    PTZ_SCHD_STRT_TIME_HEADING2,
    PTZ_SCHD_END_TIME_HEADING2,
    PTZ_SCHD_PRE_TOUR_HEADING3,
    PTZ_SCHD_OK_STR,
    PTZ_SCHD_CANCEL_STR,

    MAX_PTZ_SCHD_HEADING
}PTZ_SCHD_HEADING_e;

static const QString ptzSchdHeadings[MAX_PTZ_SCHD_HEADING] = {

    "Preset Tour Schedule",
    "Weekly Schedule",
    "Entire Day",
    "Time Period 1",
    "Time Period 2",
    "Preset Tour",
    "Start Time",
    "End Time",
    "Preset Tour",
    "Start Time",
    "End Time",
    "Preset Tour",
    "OK",
    "Cancel"

};

static const QString weekdays[] = {
    "Sunday", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday","Saturday"
};

static const quint8 weekdayMargins[] = {39,31,32,3,28,49,26};

PTZSchd::PTZSchd(QMap<quint8, QString> preList,
                 bool* enitreDay,
                 QStringList &startTime1,
                 QStringList &stopTime1,
                 QStringList &startTime2,
                 QStringList &stopTime2,
                 quint8 *presetPos, QWidget *parent) :
    KeyBoard(parent)
{
    this->setGeometry (0,0,parent->width (),parent->height ());
    presetList = preList;
    entireDaySelect = enitreDay;
    m_startTime1 = &startTime1;
    m_startTime2 = &startTime2;
    m_stopTime1 = &stopTime1;
    m_stopTime2 = &stopTime2;
    m_presetPos = presetPos;

    createDefaultComponets();
    assignIntialValues();

    currElement = PTZ_SCHD_ENTIREDAY;
    m_elementlist[currElement]->forceActiveFocus ();

    this->show ();
}

void PTZSchd::createDefaultComponets()
{
    backGround = new Rectangle((SCALE_WIDTH( SETTING_LEFT_PANEL_WIDTH) +
                               SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) - PTZ_PAGE_WIDTH)/2 ,
                               (SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)) +
                               ((SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT) - PTZ_PAGE_HEIGHT)/2 - SCALE_HEIGHT(30)),
                               PTZ_PAGE_WIDTH,
                               PTZ_PAGE_HEIGHT,
                               0,
                               BORDER_2_COLOR,
                               NORMAL_BKG_COLOR,
                               this,1);

    closeButton = new CloseButtton (backGround->x ()+ backGround->width () - SCALE_WIDTH(25),
                                    backGround->y () + SCALE_HEIGHT(20),
                                    this,
                                    CLOSE_BTN_TYPE_1,
                                    PTZ_SCHD_CLS_BTN);

    m_elementlist[PTZ_SCHD_CLS_BTN] = closeButton;

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
                          ptzSchdHeadings[PTZ_SCHD_HEADING],
                          this,
                          HEADING_TYPE_2);

    okButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                              backGround->x () + backGround->width ()/2 - SCALE_WIDTH(70),
                              backGround->y () + backGround->height () - SCALE_HEIGHT(30),
                              ptzSchdHeadings[PTZ_SCHD_OK_STR],
                              this,
                              PTZ_SCHD_OK_BTN);

    m_elementlist[PTZ_SCHD_OK_BTN] = okButton;

    connect (okButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    connect (okButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));


    cancelButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                  backGround->x () + backGround->width ()/2 + SCALE_WIDTH(70),
                                  backGround->y () + backGround->height () - SCALE_HEIGHT(30),
                                  ptzSchdHeadings[PTZ_SCHD_CANCEL_STR],
                                  this,
                                  PTZ_SCHD_CANCEL_BTN);

    m_elementlist[PTZ_SCHD_CANCEL_BTN] = cancelButton;

    connect (cancelButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    connect (cancelButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));


    eleheadings[0] = new ElementHeading(backGround->x () + (PTZ_PAGE_WIDTH - BGTILE_EXTRALARGE_SIZE_WIDTH)/2,
                                        backGround->y () + SCALE_HEIGHT(55),
                                        BGTILE_EXTRALARGE_SIZE_WIDTH,
                                        BGTILE_HEIGHT,
                                        ptzSchdHeadings[PTZ_SCHD_ELE_HEADING1],
                                        TOP_LAYER,
                                        this,
                                        false,
                                        SCALE_WIDTH(25));

    eleheadings[1] = new ElementHeading( eleheadings[0]->x (),
                                         eleheadings[0]->y () + eleheadings[0]->height (),
                                         BGTILE_EXTRALARGE_SIZE_WIDTH,
                                         BGTILE_HEIGHT,
                                         ptzSchdHeadings[PTZ_SCHD_ELE_HEADING2],
                                         MIDDLE_TABLE_LAYER,
                                         this,
                                         false,
                                         SCALE_WIDTH(150));


    eleheadings[2] = new ElementHeading( (eleheadings[0]->x () + SCALE_WIDTH(500) ),
                                         eleheadings[0]->y () + eleheadings[0]->height (),
                                         BGTILE_EXTRALARGE_SIZE_WIDTH,
                                         BGTILE_HEIGHT,
                                         ptzSchdHeadings[PTZ_SCHD_ELE_HEADING3],
                                         NO_LAYER,
                                         this,
                                         false,
                                         0);

    eleheadings[3] = new ElementHeading( (eleheadings[0]->x () + SCALE_WIDTH(960) ),
                                         eleheadings[0]->y () + eleheadings[0]->height (),
                                         BGTILE_EXTRALARGE_SIZE_WIDTH,
                                         BGTILE_HEIGHT,
                                         ptzSchdHeadings[PTZ_SCHD_ELE_HEADING4],
                                         NO_LAYER,
                                         this,
                                         false,
                                         0);

    headingTile = new BgTile( eleheadings[1]->x (),
                              eleheadings[1]->y () + eleheadings[1]->height (),
                              BGTILE_EXTRALARGE_SIZE_WIDTH,
                              BGTILE_HEIGHT,
                              MIDDLE_TABLE_LAYER,
                              this);

    quint16 textlabelsMargins[] = { 200,365,480,645,
                                    810,925,1085};

    for(quint8 index = 0; index < MAX_PTZ_WEEKDAYS; index++)
    {
        textlabels[index] = new TextLabel(headingTile->x () + SCALE_WIDTH(textlabelsMargins[index]),
                                          headingTile->y () + SCALE_HEIGHT(10),
                                          NORMAL_FONT_SIZE,
                                          ptzSchdHeadings[PTZ_SCHD_PRE_TOUR_HEADING + index],
                                          this);
    }

    for(quint8 index = 0; index < MAX_PTZ_WEEKDAYS; index ++)
    {
        entireDayWeekdaySelection[index] = new OptionSelectButton(headingTile->x (),
                                                                  headingTile->y () + (index + 1)*BGTILE_HEIGHT,
                                                                  BGTILE_EXTRALARGE_SIZE_WIDTH,
                                                                  BGTILE_HEIGHT,
                                                                  CHECK_BUTTON_INDEX,
                                                                  weekdays[index],
                                                                  this,
                                                                  ((index == (MAX_PTZ_WEEKDAYS - 1)) ? BOTTOM_TABLE_LAYER : MIDDLE_TABLE_LAYER),
                                                                  SCALE_WIDTH(weekdayMargins[index]),
                                                                  MX_OPTION_TEXT_TYPE_LABEL,
                                                                  NORMAL_FONT_SIZE,
                                                                  (PTZ_SCHD_ENTIREDAY + index));


        m_elementlist[PTZ_SCHD_ENTIREDAY +
                (index*MAX_PTZ_SCHD_ROW_ELE) + PTZ_SCHD_ENTIRE_DAY] = entireDayWeekdaySelection[index];

        connect (entireDayWeekdaySelection[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));

        connect (entireDayWeekdaySelection[index],
                 SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                 this,
                 SLOT(slotCheckBoxClick(OPTION_STATE_TYPE_e,int)));

        entireDayPresetSpinBox[index] = new DropDown( entireDayWeekdaySelection[index]->x () + SCALE_WIDTH(150),
                                                     entireDayWeekdaySelection[index]->y (),
                                                     BGTILE_SMALL_SIZE_WIDTH,
                                                     BGTILE_HEIGHT,
                                                     0,
                                                     DROPDOWNBOX_SIZE_200,
                                                     "",
                                                     presetList,
                                                     this,
                                                     "",
                                                     false,0,
                                                     NO_LAYER);

        m_elementlist[PTZ_SCHD_ENTIREDAY +
                (index*MAX_PTZ_SCHD_ROW_ELE) + PTZ_SCHD_ENT_DAY_PRE] = entireDayPresetSpinBox[index];

        connect (entireDayPresetSpinBox[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));


        timePeriod1StartTime[index] = new ClockSpinbox(entireDayPresetSpinBox[index]->x () +
                                                       entireDayPresetSpinBox[index]->width () + SCALE_WIDTH(15),
                                                       entireDayPresetSpinBox[index]->y (),
                                                       BGTILE_SMALL_SIZE_WIDTH,
                                                       BGTILE_HEIGHT,
                                                       0,
                                                       CLK_SPINBOX_With_NO_SEC,
                                                       "", 7,
                                                       this,
                                                       "",
                                                       false,0,
                                                       NO_LAYER, true, 5);

        m_elementlist[PTZ_SCHD_ENTIREDAY +
                (index*MAX_PTZ_SCHD_ROW_ELE) + PTZ_SCHD_TIME1_STRT_TIME] = timePeriod1StartTime[index];

        connect (timePeriod1StartTime[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));

        timePeriod1EndTime[index] = new ClockSpinbox(timePeriod1StartTime[index]->x () +
                                                     timePeriod1StartTime[index]->width () + SCALE_WIDTH(5),
                                                     timePeriod1StartTime[index]->y (),
                                                     BGTILE_SMALL_SIZE_WIDTH,
                                                     BGTILE_HEIGHT,
                                                     0,
                                                     CLK_SPINBOX_With_NO_SEC,
                                                     "", 7,
                                                     this,
                                                     "",
                                                     false,0,
                                                     NO_LAYER, true, 5);

        m_elementlist[PTZ_SCHD_ENTIREDAY +
                (index*MAX_PTZ_SCHD_ROW_ELE) + PTZ_SCHD_TIME1_ED_TIME] = timePeriod1EndTime[index];

        connect (timePeriod1EndTime[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));

        timePeriod1PresetSpinBox[index] = new DropDown( timePeriod1EndTime[index]->x () +
                                                       timePeriod1EndTime[index]->width (),
                                                       entireDayWeekdaySelection[index]->y (),
                                                       BGTILE_SMALL_SIZE_WIDTH,
                                                       BGTILE_HEIGHT,
                                                       0,
                                                       DROPDOWNBOX_SIZE_200,
                                                       "",
                                                       presetList,
                                                       this,
                                                       "",
                                                       false,0,
                                                       NO_LAYER);

        m_elementlist[PTZ_SCHD_ENTIREDAY +
                (index*MAX_PTZ_SCHD_ROW_ELE) + PTZ_SCHD_TIME1_PRE] = timePeriod1PresetSpinBox[index];

        connect (timePeriod1PresetSpinBox[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));

        timePeriod2StartTime[index] = new ClockSpinbox(timePeriod1PresetSpinBox[index]->x () +
                                                       timePeriod1PresetSpinBox[index]->width () + SCALE_WIDTH(15),
                                                       timePeriod1PresetSpinBox[index]->y (),
                                                       BGTILE_SMALL_SIZE_WIDTH,
                                                       BGTILE_HEIGHT,
                                                       0,
                                                       CLK_SPINBOX_With_NO_SEC,
                                                       "", 7,
                                                       this,
                                                       "",
                                                       false,0,
                                                       NO_LAYER, true, 5);
        m_elementlist[PTZ_SCHD_ENTIREDAY +
                (index*MAX_PTZ_SCHD_ROW_ELE) + PTZ_SCHD_TIME2_STRT_TIME] = timePeriod2StartTime[index];

        connect (timePeriod2StartTime[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));

        timePeriod2EndTime[index] = new ClockSpinbox(timePeriod2StartTime[index]->x () +
                                                     timePeriod2StartTime[index]->width () + SCALE_WIDTH(5),
                                                     timePeriod1StartTime[index]->y (),
                                                     BGTILE_SMALL_SIZE_WIDTH,
                                                     BGTILE_HEIGHT,
                                                     0,
                                                     CLK_SPINBOX_With_NO_SEC,
                                                     "", 7,
                                                     this,
                                                     "",
                                                     false,0,
                                                     NO_LAYER, true, 5);

        m_elementlist[PTZ_SCHD_ENTIREDAY +
                (index*MAX_PTZ_SCHD_ROW_ELE) + PTZ_SCHD_TIME2_ED_TIME] = timePeriod2EndTime[index];

        connect (timePeriod2EndTime[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));

        timePeriod2PresetSpinBox[index] = new DropDown(timePeriod2EndTime[index]->x () +
                                                       timePeriod2EndTime[index]->width () ,
                                                       entireDayWeekdaySelection[index]->y (),
                                                       BGTILE_SMALL_SIZE_WIDTH,
                                                       BGTILE_HEIGHT,
                                                       0,
                                                       DROPDOWNBOX_SIZE_200,
                                                       "",
                                                       presetList,
                                                       this,
                                                       "",
                                                       false,0,
                                                       NO_LAYER);
        m_elementlist[PTZ_SCHD_ENTIREDAY +
                (index*MAX_PTZ_SCHD_ROW_ELE) + PTZ_SCHD_TIME2_PRE] = timePeriod2PresetSpinBox[index];

        connect (timePeriod2PresetSpinBox[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));

    }

    infoPage = new InfoPage (backGround->x (),
                             backGround->y (),
                             backGround->width (),
                             backGround->height (),
                             INFO_PRESET_TOUR_SCHEDULE,
                             this);
    connect (infoPage,
             SIGNAL(sigInfoPageCnfgBtnClick(int)),
             this,
             SLOT(slotInfoPageBtnclick(int)));
}

PTZSchd::~PTZSchd()
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


    for(quint8 index = 0; index < 3; index++)
    {
        delete eleheadings[index];
    }

    delete headingTile;

    for(quint8 index = 0;index < MAX_PTZ_WEEKDAYS; index++)
    {
        delete textlabels[index];
    }

    for(quint8 index = 0; index < MAX_PTZ_WEEKDAYS; index ++)
    {

        disconnect (entireDayWeekdaySelection[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));

        disconnect (entireDayWeekdaySelection[index],
                    SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                    this,
                    SLOT(slotCheckBoxClick(OPTION_STATE_TYPE_e,int)));
        delete entireDayWeekdaySelection[index];

        disconnect (entireDayPresetSpinBox[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete entireDayPresetSpinBox[index];

        disconnect (timePeriod1StartTime[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete timePeriod1StartTime[index];

        disconnect (timePeriod1EndTime[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete timePeriod1EndTime[index];

        disconnect (timePeriod1PresetSpinBox[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete timePeriod1PresetSpinBox[index];

        disconnect (timePeriod2StartTime[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete timePeriod2StartTime[index];

        disconnect (timePeriod2EndTime[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete timePeriod2EndTime[index];

        disconnect (timePeriod2PresetSpinBox[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete timePeriod2PresetSpinBox[index];

    }

    disconnect (infoPage,
                SIGNAL(sigInfoPageCnfgBtnClick(int)),
                this,
                SLOT(slotInfoPageBtnclick(int)));
    delete infoPage;
}

void PTZSchd::assignIntialValues()
{
    QString  hour,min;

    for(quint8 index = 0; index < MAX_PTZ_WEEKDAYS; index++)
    {
        entireDayWeekdaySelection[index]->changeState
                (entireDaySelect[index] == true ? ON_STATE : OFF_STATE);

        hour = m_startTime1->at (index).mid (0,2);
        min = m_startTime1->at (index).mid (2,2);

        timePeriod1StartTime[index]->assignValue (hour,min);

        hour = m_stopTime1->at (index).mid (0,2);
        min = m_stopTime1->at (index).mid (2,2);

        timePeriod1EndTime[index]->assignValue (hour,min);

        hour = m_startTime2->at (index).mid (0,2);
        min = m_startTime2->at (index).mid (2,2);

        timePeriod2StartTime[index]->assignValue (hour,min);

        hour = m_stopTime2->at (index).mid (0,2);
        min = m_stopTime2->at (index).mid (2,2);

        timePeriod2EndTime[index]->assignValue (hour,min);

        entireDayPresetSpinBox[index]->setIndexofCurrElement (m_presetPos[(index*3)]);
        timePeriod1PresetSpinBox[index]->setIndexofCurrElement (m_presetPos[(index*3) + 1]);
        timePeriod2PresetSpinBox[index]->setIndexofCurrElement (m_presetPos[(index*3) + 2]);

        enableControls(index, entireDaySelect[index]);

    }
}

void PTZSchd::paintEvent (QPaintEvent *)
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
                                   SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT),
                                   SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH),
                                   SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)),
                             SCALE_WIDTH(RECT_RADIUS), SCALE_HEIGHT(RECT_RADIUS));
}

void PTZSchd::takeLeftKeyAction()
{
    do
    {
        currElement = (currElement - 1 + MAX_PTZ_SCHD_CTRL) %
                MAX_PTZ_SCHD_CTRL;
    }while(!m_elementlist[currElement]->getIsEnabled());

    m_elementlist[currElement]->forceActiveFocus();
}

void PTZSchd::takeRightKeyAction()
{
    do
    {
        currElement = (currElement + 1) % MAX_PTZ_SCHD_CTRL;
    }while(!m_elementlist[currElement]->getIsEnabled());

    m_elementlist[currElement]->forceActiveFocus();
}

void PTZSchd::showEvent (QShowEvent *event)
{
    QWidget::showEvent (event);
    m_elementlist[currElement]->forceActiveFocus();
}

void PTZSchd::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void PTZSchd::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    currElement = PTZ_SCHD_CLS_BTN;
    m_elementlist[currElement]->forceActiveFocus ();
}

void PTZSchd::enableControls(quint8 tempIndex,bool state)
{
    entireDayPresetSpinBox[tempIndex]->setIsEnabled (state);
    timePeriod1StartTime[tempIndex]->setIsEnabled (!state);
    timePeriod1EndTime[tempIndex]->setIsEnabled (!state);
    timePeriod1PresetSpinBox[tempIndex]->setIsEnabled (!state);
    timePeriod2StartTime[tempIndex]->setIsEnabled (!state);
    timePeriod2EndTime[tempIndex]->setIsEnabled (!state);
    timePeriod2PresetSpinBox[tempIndex]->setIsEnabled (!state);
}

bool PTZSchd::checkValidationOfTime()
{
    quint32 startTimeInMin,endTimeInMin;
    quint32 strtTimeInMin,edTimeInMin;

    quint32 starthour,startmin;
    quint32 stopHour,stopMin;

    quint32 start2hour,start2min;
    quint32 stop2Hour,stop2Min;

    QString startTime = "";
    QString stopTime = "";

    for(quint8 index = 0 ; index < MAX_PTZ_WEEKDAYS; index++)
    {
        timePeriod1StartTime[index]->currentValue (starthour,startmin);
        timePeriod1EndTime[index]->currentValue (stopHour,stopMin);

        startTimeInMin = starthour*60 + startmin ;
        endTimeInMin = stopHour*60 + stopMin ;

        if((startTimeInMin >= endTimeInMin)
                && ((startTimeInMin != 0) && (endTimeInMin != 0)))
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(GREATER_START_TM));
            return false;
        }
    }

    for(quint8 index = 0 ; index < MAX_PTZ_WEEKDAYS; index++)
    {
        timePeriod2StartTime[index]->currentValue (starthour,startmin);
        timePeriod2EndTime[index]->currentValue (stopHour,stopMin);

        startTimeInMin = starthour*60 + startmin ;
        endTimeInMin = stopHour*60 + stopMin ;

        if((startTimeInMin >= endTimeInMin)
                && ((startTimeInMin != 0) && (endTimeInMin != 0)))
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(GREATER_START_TM));
            return false;
        }
    }

    for(quint8 index = 0 ; index < MAX_PTZ_WEEKDAYS; index++)
    {
        timePeriod1StartTime[index]->currentValue (starthour,startmin);
        timePeriod1EndTime[index]->currentValue (stopHour,stopMin);

        startTimeInMin = starthour*60 + startmin ;
        endTimeInMin = stopHour*60 + stopMin ;

        if( startTimeInMin != endTimeInMin )
        {
            timePeriod2StartTime[index]->currentValue (start2hour,start2min);
            timePeriod2EndTime[index]->currentValue (stop2Hour,stop2Min);

            strtTimeInMin = start2hour*60 + start2min ;
            edTimeInMin = stop2Hour*60 + stop2Min ;

            if( strtTimeInMin != edTimeInMin )
            {
                if( ( (startTimeInMin > strtTimeInMin) && (startTimeInMin < edTimeInMin) ) ||
                        ( (strtTimeInMin > startTimeInMin) && (strtTimeInMin < endTimeInMin) ) ||
                        ( (strtTimeInMin > startTimeInMin) && (endTimeInMin == 0) ) ||
                        (startTimeInMin == strtTimeInMin)  )
                {
                    infoPage->loadInfoPage (ValidationMessage::getValidationMessage(PTZSCHD_TOUR_OVERLAP));
                    return false;
                }
            }
        }
    }

    for(quint8 index = 0 ; index < MAX_PTZ_WEEKDAYS; index++)
    {
        timePeriod1StartTime[index]->currentValue (starthour,startmin);
        timePeriod1EndTime[index]->currentValue (stopHour,stopMin);

        starthour < 10 ?
                    startTime = ( QString("0") + QString("%1").arg (starthour)) :
                startTime = ( QString("%1").arg (starthour) );

        startmin < 10 ?
                    startTime.append( QString("0") + QString("%1").arg(startmin) ) :
                    startTime.append (QString("%1").arg(startmin));

        stopHour < 10 ?
                    stopTime = ( QString("0") + QString("%1").arg (stopHour)) :
                stopTime = ( QString("%1").arg (stopHour) );

        stopMin < 10 ?
                    stopTime.append( QString("0") + QString("%1").arg(stopMin) ) :
                    stopTime.append (QString("%1").arg(stopMin));

        m_startTime1->replace (index,startTime);
        m_stopTime1->replace (index,stopTime);

        timePeriod2StartTime[index]->currentValue (starthour,startmin);
        timePeriod2EndTime[index]->currentValue (stopHour,stopMin);

        starthour < 10 ?
                    startTime = ( QString("0") + QString("%1").arg (starthour)) :
                startTime = ( QString("%1").arg (starthour) );

        startmin < 10 ?
                    startTime.append( QString("0") + QString("%1").arg(startmin) ) :
                    startTime.append (QString("%1").arg(startmin));

        stopHour < 10 ?
                    stopTime = ( QString("0") + QString("%1").arg (stopHour)) :
                stopTime = ( QString("%1").arg (stopHour) );

        stopMin < 10 ?
                    stopTime.append( QString("0") + QString("%1").arg(stopMin) ) :
                    stopTime.append (QString("%1").arg(stopMin));

        m_startTime2->replace (index,startTime);
        m_stopTime2->replace (index,stopTime);

    }
    return true;
}


void PTZSchd::slotButtonClick (int index)
{
    switch (index)
    {
    case PTZ_SCHD_OK_BTN:
    {
        if(checkValidationOfTime ())
        {
            for(quint8 index = 0 ; index < MAX_PTZ_WEEKDAYS; index++ )
            {
                entireDaySelect[index] =
                        entireDayWeekdaySelection[index]->getCurrentState () == ON_STATE ?
                            true : false;

                m_presetPos[(index*3)] = entireDayPresetSpinBox[index]->getIndexofCurrElement ();
                m_presetPos[(index*3) + 1] = timePeriod1PresetSpinBox[index]->getIndexofCurrElement ();
                m_presetPos[(index*3) + 2] = timePeriod2PresetSpinBox[index]->getIndexofCurrElement ();
            }

            emit sigObjectDel ();
        }
    }
        break;
    default:
        emit sigObjectDel ();
        break;

    }
}

void PTZSchd::slotUpdateCurrentElement (int index)
{
    currElement = index;
}

void PTZSchd::slotCheckBoxClick (OPTION_STATE_TYPE_e state, int index)
{
    enableControls((index - PTZ_SCHD_ENTIREDAY),state);
}

void PTZSchd::slotInfoPageBtnclick (int)
{
    m_elementlist[currElement]->forceActiveFocus();
}

void PTZSchd::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void PTZSchd::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void PTZSchd::insertKeyPressed(QKeyEvent *event)
{
    event->accept();
}
