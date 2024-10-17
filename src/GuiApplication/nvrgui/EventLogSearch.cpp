#include "EventLogSearch.h"
#include <QKeyEvent>
#include "ValidationMessage.h"

#define EVENT_LOG_MAIN_RECT_HEIGHT      657
#define EVENT_LOG_MAIN_RECT_WIDTH       (1340 + 35 + 60)
#define EVENT_LOG_HEADER_RECT_HEIGHT    45
#define EVENT_LOG_HEADER_RECT_WIDTH     290
#define EVENT_LOG_BGTILE_WIDTH          (1300 + 35 + 60)
#define MAX_REC_ALLOWED                 300
#define EVENT_LOG_HEADING               "Event Log"
#define EVENT_LOG_DATE_AND_TIME_WIDTH   (TABELCELL_MEDIUM_SIZE_WIDTH - SCALE_WIDTH(25))
#define EVENT_LOG_EVENT_TYPE_WIDTH      (TABELCELL_SMALL_SIZE_WIDTH  - SCALE_WIDTH(25))
#define EVENT_LOG_EVENT_WIDTH           (TABELCELL_MEDIUM_SIZE_WIDTH + SCALE_WIDTH(25))
#define EVENT_LOG_STATE_WIDTH           (TABELCELL_SMALL_SIZE_WIDTH + SCALE_WIDTH(35))
#define EVENT_LOG_SOURCE_WIDTH          (TABELCELL_MEDIUM_SIZE_WIDTH + SCALE_WIDTH(50))
#define EVENT_LOG_ADVANCE_DETAILS_WIDTH SCALE_WIDTH(320)
#define EVENT_LOG_PLAY_WIDTH            SCALE_WIDTH(60)

static const QString eventLogSearchCntrlLabel[] =
{
    "",
    "Devices",
    "Start Date",
    "",
    "End Date",
    "",
    "Type",
    "Search"
};

// cnfg field no According to CMS comm. module
typedef enum
{
    FIELD_INDEX_NO,
    FIELD_STRAT_TIME,
    FIELD_END_TIME,
    FIELD_CAM_NO,
    FIELD_EVT_TYPE,
    FIELD_OVERLAP,
    FIELD_HDD,
    FIELD_PARTION,
    MAX_FIELD_NO
}CNFG_FIELD_NO_e;

typedef enum
{
    EVNT_LOG_TABLE_INDEX,
    EVNT_LOG_TABLE_DATE_TIME,
    EVNT_LOG_TABEL_EVENT_TYPE,
    EVNT_LOG_TABLE_EVENT,
    EVNT_LOG_TABLE_STATE,
    EVNT_LOG_TABLE_SOURCE,
    EVNT_LOG_TABLE_ADVANCE_DETAILS,
    EVNT_LOG_TABLE_PLAY,
    MAX_EVNT_LOG_TABLE
}EVNT_LOG_TABLE_e;

static const QString tableHeading[] =
{
    "Index", "Date Time", "Event Type", "Event",
    "State", "Source", "Advance Details","Play"
};

static const QMap<quint8, QString> evtTypeStrMapList =
{
    {0, "All"},
    {1, "Camera"},
    {2, "Sensor"},
    {3, "Alarm"},
    {4, "System"},
    {5, "Storage"},
    {6, "Network"},
    {7, "Other"},
    {8, "User"},
    #if !defined(OEM_JCI)
    {9, "COSEC"},
    #endif
};

EventLogSearch::EventLogSearch(QWidget *parent)
    :BackGround((ApplController::getXPosOfScreen() + ((ApplController::getWidthOfScreen() - SCALE_WIDTH(EVENT_LOG_MAIN_RECT_WIDTH)) / 2)),
                (ApplController::getYPosOfScreen() + ((ApplController::getHeightOfScreen() - SCALE_HEIGHT(EVENT_LOG_MAIN_RECT_HEIGHT)) / 2)),
                SCALE_WIDTH(EVENT_LOG_MAIN_RECT_WIDTH),
                SCALE_HEIGHT(EVENT_LOG_MAIN_RECT_HEIGHT) - SCALE_HEIGHT(EVENT_LOG_HEADER_RECT_HEIGHT),
                BACKGROUND_TYPE_1,
                EVENT_LOG_BUTTON,
                parent,
                SCALE_WIDTH(EVENT_LOG_HEADER_RECT_WIDTH),
                SCALE_HEIGHT(EVENT_LOG_HEADER_RECT_HEIGHT),
                EVENT_LOG_HEADING)
{
    for(quint8 index = 0; index < MAX_EVENT_LOG_DATA_ROW; index++)
    {
        m_cameraNumber[index] = 0;
        m_currentEventNumber[index] = MAX_RECORD_TYPE;
    }
    m_currentRequestIndex = 0;
    m_totalEventIndex = 0;

    createDefaultComponents();
    getDevDateTime();

    m_actualStartTimeList.clear ();
    m_expectedEndTimeList.clear ();
    m_actualStartTimeWithSecList.clear ();

    m_currElement = EVNT_LOG_DEVLIST_SPINBOX;
    m_elementList[m_currElement]->forceActiveFocus ();

    this->show ();
}

void EventLogSearch::createDefaultComponents()
{    
    applController = ApplController::getInstance ();
    payloadLib = new PayloadLib();

    QMap<quint8, QString> deviceMapList;
    applController->GetDevNameDropdownMapList(deviceMapList);
    currentDeviceName = applController->GetRealDeviceName(deviceMapList.value(0));
    applController->GetDeviceInfo(currentDeviceName, deviceInfo);

    m_elementList[EVNT_LOG_CLOSE_BUTTON] = m_mainCloseButton;
    m_mainCloseButton->setIndexInPage(EVNT_LOG_CLOSE_BUTTON);
    connect(m_mainCloseButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));

    deviceNameDropDownBox = new DropDown((SCALE_WIDTH(EVENT_LOG_MAIN_RECT_WIDTH) - SCALE_WIDTH(EVENT_LOG_BGTILE_WIDTH))/2,
                                         (SCALE_HEIGHT(EVENT_LOG_HEADER_RECT_HEIGHT) + SCALE_HEIGHT(25)),
                                         SCALE_WIDTH(EVENT_LOG_BGTILE_WIDTH),
                                         BGTILE_HEIGHT,
                                         EVNT_LOG_DEVLIST_SPINBOX,
                                         DROPDOWNBOX_SIZE_200,
                                         eventLogSearchCntrlLabel[EVNT_LOG_DEVLIST_SPINBOX],
                                         deviceMapList,
                                         this,
                                         "",
                                         false,
                                         SCALE_WIDTH(45),
                                         UP_LAYER);
    m_elementList[EVNT_LOG_DEVLIST_SPINBOX] = deviceNameDropDownBox;
    connect(deviceNameDropDownBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));

    startDateCalender = new CalendarTile(deviceNameDropDownBox->x (),
                                         deviceNameDropDownBox->y () + deviceNameDropDownBox->height (),
                                         SCALE_WIDTH(EVENT_LOG_BGTILE_WIDTH),
                                         BGTILE_HEIGHT,
                                         "",
                                         eventLogSearchCntrlLabel[EVNT_LOG_STARTDATE_CALENDER],
                                         this,
                                         EVNT_LOG_STARTDATE_CALENDER,
                                         false,
                                         SCALE_WIDTH(22),
                                         BOTTOM_TABLE_LAYER);
    m_elementList[EVNT_LOG_STARTDATE_CALENDER] = startDateCalender;
    connect(startDateCalender,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));

    startTimeClockSpinbox = new ClockSpinbox(startDateCalender->x () + SCALE_WIDTH(288),
                                             startDateCalender->y (),
                                             SCALE_WIDTH(EVENT_LOG_BGTILE_WIDTH),
                                             BGTILE_HEIGHT,
                                             EVNT_LOG_STARTTIME_SPINBOX,
                                             CLK_SPINBOX_With_NO_SEC,
                                             eventLogSearchCntrlLabel[EVNT_LOG_STARTTIME_SPINBOX], 10,
                                             this,
                                             "",
                                             false,
                                             0, NO_LAYER);
    m_elementList[EVNT_LOG_STARTTIME_SPINBOX] = startTimeClockSpinbox;
    connect(startTimeClockSpinbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));

    endDateCalender = new CalendarTile(startDateCalender->x () + SCALE_WIDTH(420),
                                       startDateCalender->y (),
                                       SCALE_WIDTH(EVENT_LOG_BGTILE_WIDTH),
                                       BGTILE_HEIGHT,
                                       "",
                                       eventLogSearchCntrlLabel[EVNT_LOG_ENDDATE_CALENDER],
                                       this,
                                       EVNT_LOG_ENDDATE_CALENDER,
                                       false, SCALE_WIDTH(20), NO_LAYER);
    m_elementList[EVNT_LOG_ENDDATE_CALENDER] = endDateCalender;
    connect(endDateCalender,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));

    endTimeClockSpinbox = new ClockSpinbox(startDateCalender->x () + SCALE_WIDTH(700),
                                           startDateCalender->y (),
                                           BGTILE_LARGE_SIZE_WIDTH,
                                           BGTILE_HEIGHT,
                                           EVNT_LOG_ENDTIME_SPINBOX,
                                           CLK_SPINBOX_With_NO_SEC,
                                           eventLogSearchCntrlLabel[EVNT_LOG_ENDTIME_SPINBOX], 10,
                                           this, "", false, 0, NO_LAYER);
    m_elementList[EVNT_LOG_ENDTIME_SPINBOX] = endTimeClockSpinbox;
    connect(endTimeClockSpinbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));

    eventListDropDownBox = new DropDown(startDateCalender->x () + SCALE_WIDTH(850),
                                        startDateCalender->y (),
                                        SCALE_WIDTH(EVENT_LOG_BGTILE_WIDTH),
                                        BGTILE_HEIGHT,
                                        EVNT_LOG_EVENTLIST_SPINBOX,
                                        DROPDOWNBOX_SIZE_200,
                                        eventLogSearchCntrlLabel[EVNT_LOG_EVENTLIST_SPINBOX],
                                        evtTypeStrMapList,
                                        this,
                                        "",
                                        false,
                                        SCALE_WIDTH(70),
                                        NO_LAYER, true, 10);
    m_elementList[EVNT_LOG_EVENTLIST_SPINBOX] = eventListDropDownBox;
    connect(eventListDropDownBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));

    searchBtn = new ControlButton(SEARCH_BUTTON_INDEX,
                                  eventListDropDownBox->x () + eventListDropDownBox->width() + SCALE_WIDTH(180),
                                  eventListDropDownBox->y (),
                                  SCALE_WIDTH(EVENT_LOG_BGTILE_WIDTH),
                                  BGTILE_HEIGHT,
                                  this,
                                  NO_LAYER,
                                  -1,
                                  eventLogSearchCntrlLabel[EVNT_LOG_SEARCH_CTRL],
                                  true,
                                  EVNT_LOG_SEARCH_CTRL);
    m_elementList[EVNT_LOG_SEARCH_CTRL] = searchBtn;
    connect(searchBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect (searchBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    tableCellDataBg = new BgTile (startDateCalender->x (),
                                  startDateCalender->y () + startDateCalender->height () + SCALE_HEIGHT(5),
                                  SCALE_WIDTH(EVENT_LOG_BGTILE_WIDTH),
                                  (TABELCELL_HEIGHT * (MAX_EVENT_LOG_DATA_ROW + 1)) + SCALE_HEIGHT(15),
                                  COMMON_LAYER,
                                  this);

    headingTitle[EVNT_LOG_TABLE_INDEX] = new TableCell(startDateCalender->x () + SCALE_WIDTH(7),
                                                       startDateCalender->y () + startDateCalender->height () + SCALE_HEIGHT(12),
                                                       SCALE_WIDTH(65),
                                                       TABELCELL_HEIGHT,
                                                       this,
                                                       true);

    headingTitle[EVNT_LOG_TABLE_DATE_TIME] = new TableCell(headingTitle[EVNT_LOG_TABLE_INDEX]->x () +
                                                           headingTitle[EVNT_LOG_TABLE_INDEX]->width (),
                                                           headingTitle[EVNT_LOG_TABLE_INDEX]->y () ,
                                                           EVENT_LOG_DATE_AND_TIME_WIDTH,
                                                           TABELCELL_HEIGHT,
                                                           this,
                                                           true);

    headingTitle[EVNT_LOG_TABEL_EVENT_TYPE] = new TableCell(headingTitle[EVNT_LOG_TABLE_DATE_TIME]->x () +
                                                            headingTitle[EVNT_LOG_TABLE_DATE_TIME]->width (),
                                                            headingTitle[EVNT_LOG_TABLE_DATE_TIME]->y (),
                                                            EVENT_LOG_EVENT_TYPE_WIDTH,
                                                            TABELCELL_HEIGHT,
                                                            this,
                                                            true);

    headingTitle[EVNT_LOG_TABLE_EVENT] = new TableCell(headingTitle[EVNT_LOG_TABEL_EVENT_TYPE]->x () +
                                                       headingTitle[EVNT_LOG_TABEL_EVENT_TYPE]->width (),
                                                       headingTitle[EVNT_LOG_TABEL_EVENT_TYPE]->y (),
                                                       EVENT_LOG_EVENT_WIDTH,
                                                       TABELCELL_HEIGHT,
                                                       this,
                                                       true);

    headingTitle[EVNT_LOG_TABLE_STATE] = new TableCell(headingTitle[EVNT_LOG_TABLE_EVENT]->x () +
                                                       headingTitle[EVNT_LOG_TABLE_EVENT]->width (),
                                                       headingTitle[EVNT_LOG_TABLE_EVENT]->y (),
                                                       EVENT_LOG_STATE_WIDTH,
                                                       TABELCELL_HEIGHT,
                                                       this,
                                                       true);


    headingTitle[EVNT_LOG_TABLE_SOURCE] = new TableCell(headingTitle[EVNT_LOG_TABLE_STATE]->x () +
                                                        headingTitle[EVNT_LOG_TABLE_STATE]->width (),
                                                        headingTitle[EVNT_LOG_TABLE_STATE]->y (),
                                                        EVENT_LOG_SOURCE_WIDTH,
                                                        TABELCELL_HEIGHT,
                                                        this,
                                                        true);

    headingTitle[EVNT_LOG_TABLE_ADVANCE_DETAILS] = new TableCell(headingTitle[EVNT_LOG_TABLE_SOURCE]->x () +
                                                                 headingTitle[EVNT_LOG_TABLE_SOURCE]->width (),
                                                                 headingTitle[EVNT_LOG_TABLE_SOURCE]->y (),
                                                                 EVENT_LOG_ADVANCE_DETAILS_WIDTH,
                                                                 TABELCELL_HEIGHT,
                                                                 this,
                                                                 true);

    headingTitle[EVNT_LOG_TABLE_PLAY] = new TableCell(headingTitle[EVNT_LOG_TABLE_ADVANCE_DETAILS]->x () +
                                                      headingTitle[EVNT_LOG_TABLE_ADVANCE_DETAILS]->width (),
                                                      headingTitle[EVNT_LOG_TABLE_ADVANCE_DETAILS]->y (),
                                                      EVENT_LOG_PLAY_WIDTH,
                                                      TABELCELL_HEIGHT,
                                                      this,
                                                      true);

    for(quint8 index = 0; index < MAX_EVENT_LOG_DATA_COL; index++)
    {
        headingTitleLabel[index] = new TextLabel (headingTitle[index]->x () + SCALE_WIDTH(10),
                                                  (headingTitle[index]->y () +
                                                   headingTitle[index]->height ()/2),
                                                  SCALE_FONT(SUB_HEADING_FONT_SIZE),
                                                  tableHeading[index],
                                                  this,
                                                  HIGHLITED_FONT_COLOR,
                                                  NORMAL_FONT_FAMILY,
                                                  ALIGN_START_X_CENTRE_Y, 0, 0, headingTitle[index]->width(),
                                                  0, true, Qt::AlignVCenter, SCALE_WIDTH(8));
    }

    for(quint8 index = 0; index < MAX_EVENT_LOG_DATA_ROW; index++)
    {        
        indexNoTableCell[index] = new TableCell(startDateCalender->x () + SCALE_WIDTH(7),
                                                startDateCalender->y () + startDateCalender->height () + SCALE_HEIGHT(12) + (index*TABELCELL_HEIGHT) +
                                                TABELCELL_HEIGHT,
                                                TABELCELL_EXTRASMALL_SIZE_WIDTH,
                                                TABELCELL_HEIGHT,
                                                this);

        indexNoLabel[index] = new TextLabel(indexNoTableCell[index]->x () + SCALE_WIDTH(10),
                                            (indexNoTableCell[index]->y () +
                                             indexNoTableCell[index]->height ()/2),
                                            NORMAL_FONT_SIZE,
                                            "",
                                            this,
                                            NORMAL_FONT_COLOR,
                                            NORMAL_FONT_FAMILY,
                                            ALIGN_START_X_CENTRE_Y, 0, 0, TABELCELL_EXTRASMALL_SIZE_WIDTH);

        dateAndTimeTabelCell[index] = new TableCell(headingTitle[EVNT_LOG_TABLE_INDEX]->x () +
                                                    headingTitle[EVNT_LOG_TABLE_INDEX]->width (),
                                                    headingTitle[EVNT_LOG_TABLE_INDEX]->y () +
                                                    (index*headingTitle[EVNT_LOG_TABLE_INDEX]->height ()) +
                                                    TABELCELL_HEIGHT,
                                                    EVENT_LOG_DATE_AND_TIME_WIDTH,
                                                    TABELCELL_HEIGHT,
                                                    this);

        dateAndTimeLabel[index] = new TextLabel(dateAndTimeTabelCell[index]->x () + SCALE_WIDTH(10),
                                                (dateAndTimeTabelCell[index]->y () +
                                                 dateAndTimeTabelCell[index]->height ()/2),
                                                NORMAL_FONT_SIZE,
                                                "",
                                                this,
                                                NORMAL_FONT_COLOR,
                                                NORMAL_FONT_FAMILY,
                                                ALIGN_START_X_CENTRE_Y, 0, 0, EVENT_LOG_DATE_AND_TIME_WIDTH,
                                                0, true, Qt::AlignVCenter, SCALE_WIDTH(8));

        eventTypeTabelCell[index] = new TableCell(headingTitle[EVNT_LOG_TABLE_DATE_TIME]->x () +
                                                  headingTitle[EVNT_LOG_TABLE_DATE_TIME]->width (),
                                                  headingTitle[EVNT_LOG_TABLE_DATE_TIME]->y () +
                                                  (index*headingTitle[EVNT_LOG_TABLE_DATE_TIME]->height ()) +
                                                  TABELCELL_HEIGHT,
                                                  EVENT_LOG_EVENT_TYPE_WIDTH,
                                                  TABELCELL_HEIGHT,
                                                  this);

        eventTypeLabel[index] = new TextLabel(eventTypeTabelCell[index]->x () + SCALE_WIDTH(10),
                                              (eventTypeTabelCell[index]->y () +
                                               eventTypeTabelCell[index]->height ()/2),
                                              NORMAL_FONT_SIZE,
                                              "",
                                              this,
                                              NORMAL_FONT_COLOR,
                                              NORMAL_FONT_FAMILY,
                                              ALIGN_START_X_CENTRE_Y, 0, 0, EVENT_LOG_EVENT_TYPE_WIDTH,
                                              0, true, Qt::AlignVCenter, SCALE_WIDTH(8));

        eventTabelCell[index] = new TableCell(headingTitle[EVNT_LOG_TABEL_EVENT_TYPE]->x () +
                                              headingTitle[EVNT_LOG_TABEL_EVENT_TYPE]->width (),
                                              headingTitle[EVNT_LOG_TABEL_EVENT_TYPE]->y () +
                                              (index*headingTitle[EVNT_LOG_TABEL_EVENT_TYPE]->height ()) +
                                              TABELCELL_HEIGHT,
                                              EVENT_LOG_EVENT_WIDTH,
                                              TABELCELL_HEIGHT,
                                              this);

        eventLabel[index] = new TextLabel(eventTabelCell[index]->x () + SCALE_WIDTH(10),
                                          (eventTabelCell[index]->y () +
                                           eventTabelCell[index]->height ()/2),
                                          NORMAL_FONT_SIZE,
                                          "",
                                          this,
                                          NORMAL_FONT_COLOR,
                                          NORMAL_FONT_FAMILY,
                                          ALIGN_START_X_CENTRE_Y, 0, 0, EVENT_LOG_EVENT_WIDTH,
                                          0, true, Qt::AlignVCenter, SCALE_WIDTH(8));

        stateTabelCell[index] = new TableCell(headingTitle[EVNT_LOG_TABLE_EVENT]->x () +
                                              headingTitle[EVNT_LOG_TABLE_EVENT]->width (),
                                              headingTitle[EVNT_LOG_TABLE_EVENT]->y () +
                                              (index*headingTitle[EVNT_LOG_TABLE_EVENT]->height ()) +
                                              TABELCELL_HEIGHT,
                                              EVENT_LOG_STATE_WIDTH,
                                              TABELCELL_HEIGHT,
                                              this);

        stateLabel[index] = new TextLabel(stateTabelCell[index]->x () + SCALE_WIDTH(10),
                                          (stateTabelCell[index]->y () +
                                           stateTabelCell[index]->height ()/2),
                                          NORMAL_FONT_SIZE,
                                          "",
                                          this,
                                          NORMAL_FONT_COLOR,
                                          NORMAL_FONT_FAMILY,
                                          ALIGN_START_X_CENTRE_Y, 0, 0, EVENT_LOG_STATE_WIDTH,
                                          0, true, Qt::AlignVCenter, SCALE_WIDTH(8));

        sourceTabelCell[index] = new TableCell(headingTitle[EVNT_LOG_TABLE_STATE]->x () +
                                               headingTitle[EVNT_LOG_TABLE_STATE]->width (),
                                               headingTitle[EVNT_LOG_TABLE_STATE]->y () +
                                               (index*headingTitle[EVNT_LOG_TABLE_STATE]->height ()) +
                                               TABELCELL_HEIGHT,
                                               EVENT_LOG_SOURCE_WIDTH,
                                               TABELCELL_HEIGHT,
                                               this);

        sourceLabel[index] = new TextLabel(sourceTabelCell[index]->x () + SCALE_WIDTH(10),
                                           (sourceTabelCell[index]->y () +
                                            sourceTabelCell[index]->height ()/2),
                                           NORMAL_FONT_SIZE,
                                           "",
                                           this,
                                           NORMAL_FONT_COLOR,
                                           NORMAL_FONT_FAMILY,
                                           ALIGN_START_X_CENTRE_Y, 0, 0, EVENT_LOG_SOURCE_WIDTH,
                                           0, true, Qt::AlignVCenter, SCALE_WIDTH(8));

        advanceDetailTabelCell[index] = new TableCell(headingTitle[EVNT_LOG_TABLE_SOURCE]->x () +
                                                      headingTitle[EVNT_LOG_TABLE_SOURCE]->width (),
                                                      headingTitle[EVNT_LOG_TABLE_SOURCE]->y () +
                                                      (index*headingTitle[EVNT_LOG_TABLE_SOURCE]->height ()) +
                                                      TABELCELL_HEIGHT,
                                                      EVENT_LOG_ADVANCE_DETAILS_WIDTH,
                                                      TABELCELL_HEIGHT,
                                                      this);

        advanceDetailLabel[index] = new TextLabel(advanceDetailTabelCell[index]->x () + SCALE_WIDTH(10),
                                                  (advanceDetailTabelCell[index]->y () +
                                                   advanceDetailTabelCell[index]->height ()/2),
                                                  NORMAL_FONT_SIZE,
                                                  "",
                                                  this,
                                                  NORMAL_FONT_COLOR,
                                                  NORMAL_FONT_FAMILY,
                                                  ALIGN_START_X_CENTRE_Y,
                                                  0,
                                                  false,
                                                  EVENT_LOG_ADVANCE_DETAILS_WIDTH,
                                                  0, true, Qt::AlignVCenter, SCALE_WIDTH(8));

        playButtonTabelCell[index] = new TableCell(headingTitle[EVNT_LOG_TABLE_ADVANCE_DETAILS]->x () +
                                                   headingTitle[EVNT_LOG_TABLE_ADVANCE_DETAILS]->width (),
                                                   headingTitle[EVNT_LOG_TABLE_ADVANCE_DETAILS]->y () +
                                                   (index*headingTitle[EVNT_LOG_TABLE_ADVANCE_DETAILS]->height ()) +
                                                   TABELCELL_HEIGHT,
                                                   EVENT_LOG_PLAY_WIDTH,
                                                   TABELCELL_HEIGHT,
                                                   this);

        playControlButton[index] = new ControlButton(PLAY_BUTTON_INDEX,
                                                     playButtonTabelCell[index]->x () + SCALE_WIDTH(15),
                                                     playButtonTabelCell[index]->y () - SCALE_HEIGHT(5),
                                                     EVENT_LOG_PLAY_WIDTH,
                                                     BGTILE_HEIGHT,
                                                     this, NO_LAYER, 0,
                                                     "", false,
                                                     (EVNT_LOG_PLAY_CTRL + index));
        m_elementList[(EVNT_LOG_PLAY_CTRL + index)] =  playControlButton[index];
        connect (playControlButton[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpadateCurrentElement(int)));
        connect (playControlButton[index],
                 SIGNAL(sigButtonClick(int)),
                 this,
                 SLOT(slotButtonClick(int)));

        playControlButton[index]->setVisible (false);
    }

    m_firstEventIndex = 0;
    m_lastEventIndex = (MAX_EVENT_LOG_DATA_ROW - 1);
    eventScrollbar = new ScrollBar((playButtonTabelCell[0]->x() + playButtonTabelCell[0]->width() - SCALE_WIDTH(13)),
                                   (playButtonTabelCell[0]->y()),
                                   SCALE_WIDTH(13),
                                   MAX_EVENT_LOG_DATA_ROW,
                                   TABELCELL_HEIGHT,
                                   EVENT_LOG_ENTRY_MAX,
                                   m_firstEventIndex,
                                   this,
                                   VERTICAL_SCROLLBAR,
                                   EVNT_LOG_SCROLL_CTRL,
                                   false,
                                   false);
    m_elementList[EVNT_LOG_SCROLL_CTRL] = eventScrollbar;
    connect(eventScrollbar,
            SIGNAL(sigScroll(int)),
            this,
            SLOT(slotEventScrollbarClick(int)));
    eventScrollbar->setIsEnabled(false);
    eventScrollbar->setVisible(false);

    m_liveEventParser = new LiveEventParser();

    processBar = new ProcessBar(0, SCALE_HEIGHT(EVENT_LOG_HEADER_RECT_HEIGHT),
                                SCALE_WIDTH(EVENT_LOG_MAIN_RECT_WIDTH),
                                (SCALE_HEIGHT(EVENT_LOG_MAIN_RECT_HEIGHT) - SCALE_HEIGHT(EVENT_LOG_HEADER_RECT_HEIGHT)),
                                SCALE_WIDTH(15), this);

    infoPage = new InfoPage(0, 0,
                            SCALE_WIDTH(EVENT_LOG_MAIN_RECT_WIDTH),
                            (SCALE_HEIGHT(EVENT_LOG_MAIN_RECT_HEIGHT) + SCALE_HEIGHT(EVENT_LOG_HEADER_RECT_HEIGHT)),
                            INFO_EVENTLOG,
                            this);
    connect (infoPage,
             SIGNAL(sigInfoPageCnfgBtnClick(int)),
             this,
             SLOT(slotInfoPageCnfgBtnClick(int)));
}

EventLogSearch::~EventLogSearch()
{
    m_logEvtInfo.clear();
    disconnect(m_mainCloseButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete payloadLib;

    m_eventList.clear ();
    delete m_liveEventParser;
    delete processBar;

    disconnect (infoPage,
                SIGNAL(sigInfoPageCnfgBtnClick(int)),
                this,
                SLOT(slotInfoPageCnfgBtnClick(int)));
    delete infoPage;

    disconnect(deviceNameDropDownBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete deviceNameDropDownBox;

    disconnect(startDateCalender,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete startDateCalender;

    disconnect(startTimeClockSpinbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete startTimeClockSpinbox;

    disconnect(endDateCalender,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete endDateCalender;

    disconnect(endTimeClockSpinbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete endTimeClockSpinbox;

    disconnect(eventListDropDownBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete eventListDropDownBox;

    disconnect(searchBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));

    disconnect (searchBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    delete searchBtn;

    for(quint8 index = 0; index < MAX_EVENT_LOG_DATA_COL; index++)
    {
        delete headingTitle[index];
        delete headingTitleLabel[index];
    }

    delete tableCellDataBg;
    for(quint8 index = 0; index < MAX_EVENT_LOG_DATA_ROW; index++)
    {
        delete indexNoTableCell[index];
        delete indexNoLabel[index];
        delete dateAndTimeTabelCell[index];
        delete dateAndTimeLabel[index];
        delete eventTypeTabelCell[index];
        delete eventTypeLabel[index];
        delete eventTabelCell[index];
        delete eventLabel[index];
        delete stateTabelCell[index];
        delete stateLabel[index];
        delete sourceTabelCell[index];
        delete sourceLabel[index];
        delete advanceDetailTabelCell[index];
        delete advanceDetailLabel[index];
        delete playButtonTabelCell[index];
        disconnect (playControlButton[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpadateCurrentElement(int)));
        disconnect (playControlButton[index],
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotButtonClick(int)));
        delete playControlButton[index];
    }

    disconnect(eventScrollbar,
               SIGNAL(sigScroll(int)),
               this,
               SLOT(slotEventScrollbarClick(int)));
    DELETE_OBJ(eventScrollbar);
}

void EventLogSearch::clearSearchRecList(quint8 index)
{
    for(; index < MAX_EVENT_LOG_DATA_ROW; index++)
    {
        indexNoLabel[index]->changeText ("");
        dateAndTimeLabel[index]->changeText ("");
        eventTypeLabel[index]->changeText ("");
        eventLabel[index]->changeText ("");
        stateLabel[index]->changeText ("");
        sourceLabel[index]->changeText ("");
        advanceDetailLabel[index]->changeText ("");
        playControlButton[index]->setVisible (false);
        playControlButton[index]->setIsEnabled (false);
    }
}

void EventLogSearch::getDevDateTime()
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = GET_DATE_TIME;
    applController->processActivity(currentDeviceName, DEVICE_COMM, param);
}

void EventLogSearch::searchEventData()
{
    DDMMYY_PARAM_t *dateParam;
    quint32 tempHr, tempMin;
    QDate Date;
    QTime Time;
    QDateTime startDateTime, endDateTime;

    dateParam = startDateCalender->getDDMMYY ();
    startTimeClockSpinbox->currentValue (tempHr, tempMin);

    Date.setDate (dateParam->year, dateParam->month, dateParam->date);
    Time.setHMS (tempHr, tempMin, 0);

    startDateTime.setDate (Date);
    startDateTime.setTime (Time);

    dateParam = endDateCalender->getDDMMYY ();
    endTimeClockSpinbox->currentValue (tempHr, tempMin);

    Date.setDate (dateParam->year, dateParam->month, dateParam->date);
    Time.setHMS (tempHr, tempMin, 0);
    endDateTime.setDate (Date);
    endDateTime.setTime (Time);

    if(startDateTime >= endDateTime)
    {
        defaultState ();
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(START_END_DATE_ERR_MSG));
    }
    else if(((endDateTime.toTime_t () - startDateTime.toTime_t ())/(60*60*24)) > 30)
    {
        defaultState ();
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(DATE_DIFF_ERR_MSG));
    }
    else
    {
        payloadLib->setCnfgArrayAtIndex (0, startDateTime.toString ("ddMMyyyyHHmm"));
        payloadLib->setCnfgArrayAtIndex (1, endDateTime.toString ("ddMMyyyyHHmm"));
        payloadLib->setCnfgArrayAtIndex (2, eventListDropDownBox->getIndexofCurrElement ());
        payloadLib->setCnfgArrayAtIndex (3, EVENT_LOG_ENTRY_MAX);

        DevCommParam* param = new DevCommParam();
        param->msgType = MSG_SET_CMD;
        param->cmdType = SEARCH_STR_EVT;
        param->payload = payloadLib->createDevCmdPayload(4);

        currentDeviceName = applController->GetRealDeviceName(deviceNameDropDownBox->getCurrValue());
        applController->GetDeviceInfo(currentDeviceName, deviceInfo);
        processBar->loadProcessBar ();
        applController->processActivity(currentDeviceName, DEVICE_COMM, param);
    }
}

void EventLogSearch::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    PlaybackRecordData playbackRecData;
    quint8 tempResponseCount = 0;
    quint8 index = 0;

    if((deviceName != currentDeviceName) || (param->msgType != MSG_SET_CMD))
    {
        return;
    }

    switch(param->deviceStatus)
    {
        case CMD_MORE_EVENTS:
        case CMD_SUCCESS:
        {
            switch(param->cmdType)
            {
                case GET_DATE_TIME:
                {
                    DDMMYY_PARAM_t dateTimeParam;

                    payloadLib->parseDevCmdReply(true, param->payload);

                    QDateTime dt = QDateTime::fromString (payloadLib->getCnfgArrayAtIndex(0).toString(), "ddMMyyyyHHmmss");
                    dt = dt.addSecs (60);
                    QDate date = dt.date ();
                    QTime time = dt.time ();

                    dateTimeParam.date  = date.day ();
                    dateTimeParam.month = date.month ();
                    dateTimeParam.year  = date.year ();
                    endDateCalender->setDDMMYY (&dateTimeParam);
                    endTimeClockSpinbox->assignValue (time.hour (), time.minute ());

                    // show start time 1 Hr Before
                    dt = dt.addSecs (-3600);
                    date = dt.date ();
                    time = dt.time ();
                    dateTimeParam.date  = date.day ();
                    dateTimeParam.month = date.month ();
                    dateTimeParam.year  = date.year ();
                    startDateCalender->setDDMMYY (&dateTimeParam);
                    startTimeClockSpinbox->assignValue (time.hour (), time.minute ());
                }
                break;

                case SEARCH_STR_EVT:
                {
                    // payloadLib passes to liveEventPraser it parse the data as per the eventData feilds of given index payload
                    QString regularExpressionStringRecord = "";
                    regularExpressionStringRecord.append('[');
                    regularExpressionStringRecord.append(SOI);
                    regularExpressionStringRecord.append(EOI);
                    regularExpressionStringRecord.append(']');
                    QRegExp regExpForRecord(regularExpressionStringRecord);
                    m_eventList.clear();
                    m_eventList = param->payload.split (regExpForRecord, QString::SkipEmptyParts);
                    m_totalEventIndex = m_eventList.length ();
                    storeEventInfo();
                    if (m_totalEventIndex > MAX_EVENT_LOG_DATA_ROW)
                    {
                        eventScrollbar->setIsEnabled(true);
                        eventScrollbar->setVisible(true);
                        m_lastEventIndex = MAX_EVENT_LOG_DATA_ROW-1;
                    }
                    else
                    {
                        eventScrollbar->setIsEnabled(false);
                        eventScrollbar->setVisible(false);
                        m_lastEventIndex = m_totalEventIndex-1;
                    }

                    m_firstEventIndex = 0;
                    eventScrollbar->updateTotalElement(m_totalEventIndex);
                    fillRecordList();

                    processBar->unloadProcessBar();
                    if(param->deviceStatus == CMD_MORE_EVENTS)
                    {
                        infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                    }
                }
                break;

                case SEARCH_RCD_DATA:
                {
                    processBar->unloadProcessBar ();
                    payloadLib->parseDevCmdReply(false, param->payload);

                    tempResponseCount = (payloadLib->getTotalCmdFields ()/ MAX_FIELD_NO);
                    if(tempResponseCount > 1)
                    {
                        quint8 tempIndex = index;
                        QString refTimeCntStr = m_actualStartTimeWithSecList.at (m_currentRequestIndex);
                        qlonglong refTime = refTimeCntStr.toLongLong();
                        qlonglong absLowestDistance = refTime;

                        for (; index< tempResponseCount ; index++)
                        {
                            qlonglong diff = qAbs(refTime - payloadLib->getCnfgArrayAtIndex (FIELD_STRAT_TIME + (index *MAX_FIELD_NO)).toLongLong());
                            if(diff < absLowestDistance)
                            {
                                absLowestDistance = diff;
                                tempIndex = index;
                            }
                        }

                        index = tempIndex;
                    }

                    playbackRecData.startTime =  payloadLib->getCnfgArrayAtIndex (FIELD_STRAT_TIME + (index *MAX_FIELD_NO)).toString ();
                    playbackRecData.endTime =  payloadLib->getCnfgArrayAtIndex (FIELD_END_TIME + (index *MAX_FIELD_NO)).toString ();
                    playbackRecData.camNo =  payloadLib->getCnfgArrayAtIndex (FIELD_CAM_NO + (index *MAX_FIELD_NO)).toUInt ();
                    playbackRecData.evtType =  payloadLib->getCnfgArrayAtIndex (FIELD_EVT_TYPE + (index *MAX_FIELD_NO)).toUInt ();
                    playbackRecData.overlap =  payloadLib->getCnfgArrayAtIndex (FIELD_OVERLAP + (index *MAX_FIELD_NO)).toUInt ();
                    playbackRecData.hddIndicator =  payloadLib->getCnfgArrayAtIndex (FIELD_HDD + (index *MAX_FIELD_NO)).toUInt ();
                    playbackRecData.partionIndicator =  payloadLib->getCnfgArrayAtIndex (FIELD_PARTION + (index *MAX_FIELD_NO)).toUInt ();
                    playbackRecData.deviceName = currentDeviceName;

                    // It is play btn click, so emit sig to Layout to start playback
                    emit sigPlaybackPlayBtnClick (playbackRecData, currentDeviceName, false);
                }
                break;

                default:
                {
                    processBar->unloadProcessBar ();
                    if(param->cmdType != SEARCH_RCD_DATA)
                    {
                        defaultState();
                    }
                    infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                }
                break;
            }
        }
        break;

        default:
        {
            processBar->unloadProcessBar ();
            if(param->cmdType != SEARCH_RCD_DATA)
            {
                defaultState();
            }
            infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
        }
        break;
    }
}


void EventLogSearch::defaultState()
{
    m_eventList.clear();
    clearSearchRecList(0);
    eventScrollbar->setIsEnabled(false);
    eventScrollbar->setVisible(false);
    m_firstEventIndex = 0;
    m_lastEventIndex = (MAX_EVENT_LOG_DATA_ROW-1);
}

void EventLogSearch::storeEventInfo()
{
    m_logEvtInfo.clear();
    LOG_EVENT_INFO_t tLogEvtInfo;
    //filling list info in Asending order(Old to latest)
    for(quint16 index = 0; index < m_totalEventIndex; index++)
    {
        m_liveEventParser->setEventString(m_eventList.at(index), true);

        tLogEvtInfo.dateTime = m_liveEventParser->getDateAndTime();
        tLogEvtInfo.eventType = m_liveEventParser->getEventType();
        tLogEvtInfo.eventSubType = m_liveEventParser->getEventSubType();
        tLogEvtInfo.eventState = m_liveEventParser->getEventState();
        tLogEvtInfo.eventSource = m_liveEventParser->getEventSource();
        tLogEvtInfo.eventAdvanceDetail = m_liveEventParser->getEventAdvanceDetail();
        tLogEvtInfo.eventTypeIndex = m_liveEventParser->getEventTypeIndex();
        tLogEvtInfo.eventSubTypeIndex = m_liveEventParser->getEventSubTypeIndex();
        tLogEvtInfo.eventStateIndex = m_liveEventParser->getEventStateIndex();
        tLogEvtInfo.actualEventDateTime = m_liveEventParser->getActualDateAndTime();
        tLogEvtInfo.actualEventDateTimeWithSec = m_liveEventParser->getActualDateAndTimeWithSec();
        tLogEvtInfo.expectedDateAndTime = m_liveEventParser->getExpectedDateAndTime();
        tLogEvtInfo.cameraNumber = m_liveEventParser->getCameraNumber();

        m_logEvtInfo.append(tLogEvtInfo);
    }
}

void EventLogSearch::fillRecordList()
{
    quint8 index;
    quint16 eventIndex;
    LOG_EVENT_INFO_t tlogEvtInfo;

    m_actualStartTimeList.clear ();
    m_expectedEndTimeList.clear ();
    m_actualStartTimeWithSecList.clear ();

    for(index = 0; index < MAX_EVENT_LOG_DATA_ROW; index++)
    {
        //filling list Asending order(Old to latest)
        eventIndex = index + m_firstEventIndex;
        if (eventIndex > m_lastEventIndex)
        {
            break;
        }

        tlogEvtInfo = m_logEvtInfo.at(eventIndex);

        if (((tlogEvtInfo.eventTypeIndex== LOG_CAMERA_EVENT) &&
             ((tlogEvtInfo.eventSubTypeIndex != LOG_CONNECTIVITY) &&
              (tlogEvtInfo.eventSubTypeIndex != LOG_PRESET_TOUR) &&
              (tlogEvtInfo.eventSubTypeIndex != LOG_NO_CAMERA_EVENT)) &&
             (tlogEvtInfo.eventSubTypeIndex != LOG_SNAPSHOT_SCHEDULE) &&
             (tlogEvtInfo.eventStateIndex == EVENT_START)))
        {
            playControlButton[index]->setIsEnabled (true);
            switch(tlogEvtInfo.eventSubTypeIndex)
            {
                case LOG_MANUAL_RECORDING:
                    m_currentEventNumber[index] = RECORD_TYPE_MANUAL;
                    break;

                case LOG_ALARM_RECORDING:
                    m_currentEventNumber[index] = RECORD_TYPE_ALARM;
                    break;

                case LOG_SCHEDULE_RECORDING:
                    m_currentEventNumber[index] = RECORD_TYPE_SCHEDULE;
                    break;

                default:
                    m_currentEventNumber[index] = RECORD_TYPE_ALL;
                    break;
            }
        }
        // COSEC Recording
        else if(((tlogEvtInfo.eventTypeIndex == LOG_COSEC_EVENT) &&
                 (tlogEvtInfo.eventSubTypeIndex == LOG_COSEC_RECORDING) &&
                 (tlogEvtInfo.eventStateIndex == EVENT_START)))
        {
            playControlButton[index]->setIsEnabled (true);
            m_currentEventNumber[index] = RECORD_TYPE_COSEC;
        }
        else
        {
            playControlButton[index]->setVisible (false);
            playControlButton[index]->setIsEnabled (false);
            m_currentEventNumber[index] = (RECORD_TYPE_e)0;
        }

        if(deviceInfo.recordFormatType == REC_AVI_FORMAT)
        {
            playControlButton[index]->setVisible (false);
            playControlButton[index]->setIsEnabled (false);
        }

        indexNoLabel[index]->changeText (QString("%1").arg (eventIndex + 1));
        dateAndTimeLabel[index]->changeText (tlogEvtInfo.dateTime);
        eventTypeLabel[index]->changeText (tlogEvtInfo.eventType);
        eventLabel[index]->changeText (tlogEvtInfo.eventSubType);
        stateLabel[index]->changeText (tlogEvtInfo.eventState);
        sourceLabel[index]->changeText (tlogEvtInfo.eventSource);
        advanceDetailLabel[index]->changeText (tlogEvtInfo.eventAdvanceDetail);
        m_actualStartTimeList.insert (index,tlogEvtInfo.actualEventDateTime);
        m_expectedEndTimeList.insert (index,tlogEvtInfo.expectedDateAndTime);
        m_actualStartTimeWithSecList.insert (index,tlogEvtInfo.actualEventDateTimeWithSec);
        m_cameraNumber[index] = tlogEvtInfo.cameraNumber;
    }

    clearSearchRecList(index);
    update ();
}

void EventLogSearch::searchCameraRecording (quint8 index)
{
    m_currentRequestIndex = index;

    payloadLib->setCnfgArrayAtIndex (0, m_cameraNumber[index]);
    payloadLib->setCnfgArrayAtIndex (1, m_currentEventNumber[index]);
    payloadLib->setCnfgArrayAtIndex (2, m_actualStartTimeList.at (index));
    payloadLib->setCnfgArrayAtIndex (3, m_expectedEndTimeList.at (index));
    payloadLib->setCnfgArrayAtIndex (4, MAX_REC_ALLOWED);

    //when we give MAX_RECORDING_STORAGE_DRIVE in search parameter, server will search in current storage recording drive
    payloadLib->setCnfgArrayAtIndex (5, MAX_RECORDING_STORAGE_DRIVE);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = SEARCH_RCD_DATA;
    param->payload = payloadLib->createDevCmdPayload(6);

    processBar->loadProcessBar ();
    applController->processActivity(currentDeviceName, DEVICE_COMM, param);
}

void EventLogSearch::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void EventLogSearch::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void EventLogSearch::navigationKeyPressed (QKeyEvent *event)
{
    event->accept();
}

void EventLogSearch::escKeyPressed (QKeyEvent *event)
{
    event->accept();
    if(!m_mainCloseButton->hasFocus ())
    {
        m_elementList[EVNT_LOG_CLOSE_BUTTON]->forceActiveFocus ();
    }
}

void EventLogSearch::wheelEvent(QWheelEvent * event)
{
    if((eventScrollbar != NULL) && (eventScrollbar->getIsEnabled() == true) && (eventScrollbar->isVisible() == true)
            && (event->x() >= indexNoTableCell[0]->x())
            && (event->x() <= (eventScrollbar->x() + eventScrollbar->width()))
            && (event->y() >= indexNoTableCell[0]->y())
            && (event->y() <= (eventScrollbar->y() + eventScrollbar->height())))
    {
        eventScrollbar->wheelEvent(event);
    }
    else
    {
        QWidget::wheelEvent(event);
    }
}

void EventLogSearch::showEvent (QShowEvent *event)
{
    QWidget::showEvent (event);
    m_elementList[m_currElement]->forceActiveFocus ();
}

void EventLogSearch::takeLeftKeyAction()
{
    do
    {
        m_currElement = (m_currElement - 1 + MAX_EVNT_LOG_CTRL) % MAX_EVNT_LOG_CTRL;
    }while(!m_elementList[m_currElement]->getIsEnabled());

    m_elementList[m_currElement]->forceActiveFocus();
}

void EventLogSearch::takeRightKeyAction()
{
    do
    {
        m_currElement = (m_currElement + 1) % MAX_EVNT_LOG_CTRL;
    }while(!m_elementList[m_currElement]->getIsEnabled());

    m_elementList[m_currElement]->forceActiveFocus();
}

void EventLogSearch::slotButtonClick (int index)
{
    switch(index)
    {
        case EVNT_LOG_SEARCH_CTRL:
        {
            searchEventData();
        }
        break;

        default:
        {
            if (index < EVNT_LOG_PLAY_CTRL)
            {
                break;
            }

            index = index - EVNT_LOG_PLAY_CTRL;
            if (index < MAX_EVENT_LOG_DATA_ROW)
            {
                searchCameraRecording(index);
            }
        }
        break;
    }
}

void EventLogSearch::slotInfoPageCnfgBtnClick(int)
{
    m_elementList[m_currElement]->forceActiveFocus ();
}

void EventLogSearch::slotUpadateCurrentElement (int index)
{
    m_currElement = index;
}

void EventLogSearch::slotEventScrollbarClick(int numberOfSteps)
{
    if ((m_lastEventIndex + numberOfSteps) > (m_totalEventIndex - 1))
    {
        numberOfSteps = (m_totalEventIndex - 1) - m_lastEventIndex;
    }

    if ((m_firstEventIndex + numberOfSteps) < 0)
    {
        numberOfSteps = -m_firstEventIndex;
    }

    m_firstEventIndex += numberOfSteps;
    m_lastEventIndex += numberOfSteps;
    fillRecordList();
}

void EventLogSearch::updateDeviceList(void)
{
    QMap<quint8, QString> deviceMapList;
    applController->GetDevNameDropdownMapList(deviceMapList);

    /* Check if selected device found in new updated list or not. It is possible that index of that device name may changed.
     * Hence update device list with current device index */
    for (quint8 deviceIndex = 0; deviceIndex < deviceMapList.count(); deviceIndex++)
    {
        if (deviceNameDropDownBox->getCurrValue() == deviceMapList.value(deviceIndex))
        {
            deviceNameDropDownBox->setNewList(deviceMapList, deviceIndex);
            return;
        }
    }

    /* If selected device is local device then it will update the device name only otherwise it will clear the data and will select the local device */
    deviceNameDropDownBox->setNewList(deviceMapList, 0);
    if (currentDeviceName != applController->GetRealDeviceName(deviceMapList.value(0)))
    {
        currentDeviceName = applController->GetRealDeviceName(deviceMapList.value(0));
        defaultState();
    }
}
