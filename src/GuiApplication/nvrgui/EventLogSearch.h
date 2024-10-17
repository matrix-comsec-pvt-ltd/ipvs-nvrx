#ifndef EVENTLOGSEARCH_H
#define EVENTLOGSEARCH_H

#include <QWidget>
#include "ApplController.h"
#include "PayloadLib.h"
#include "NavigationControl.h"

#include "Controls/BackGround.h"
#include "Controls/DropDown.h"
#include "Controls/CalendarTile.h"
#include "Controls/Clockspinbox.h"
#include "Controls/ControlButton.h"
#include "Controls/TableCell.h"
#include "Controls/Heading.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/ReadOnlyElement.h"
#include "Controls/CnfgButton.h"
#include "Controls/InfoPage.h"
#include "Controls/ProcessBar.h"
#include "LiveEventParser.h"
#include "Controls/PlaybackRecordData.h"
#include "Controls/TextLabel.h"

#define MAX_EVENT_LOG_DATA_COL  8
#define MAX_EVENT_LOG_DATA_ROW  14
#define EVENT_LOG_ENTRY_MAX     2000

typedef enum
{
    EVNT_LOG_CLOSE_BUTTON,
    EVNT_LOG_DEVLIST_SPINBOX,
    EVNT_LOG_STARTDATE_CALENDER,
    EVNT_LOG_STARTTIME_SPINBOX,
    EVNT_LOG_ENDDATE_CALENDER,
    EVNT_LOG_ENDTIME_SPINBOX,
    EVNT_LOG_EVENTLIST_SPINBOX,
    EVNT_LOG_SEARCH_CTRL,
    EVNT_LOG_PLAY_CTRL,
    EVNT_LOG_SCROLL_CTRL = (EVNT_LOG_PLAY_CTRL + MAX_EVENT_LOG_DATA_ROW),
    MAX_EVNT_LOG_CTRL,
}EVNT_LOG_CTRL_e;

typedef enum
{
    RECORD_TYPE_ALL = 15,
    RECORD_TYPE_MANUAL = 1,
    RECORD_TYPE_ALARM = 2,
    RECORD_TYPE_SCHEDULE = 4,
    RECORD_TYPE_COSEC = 8,
    MAX_RECORD_TYPE
}RECORD_TYPE_e;

typedef struct
{
    QString                 dateTime;
    QString                 eventType;
    QString                 eventSubType;
    QString                 eventState;
    QString                 eventSource;
    QString                 eventAdvanceDetail;
    LOG_EVENT_TYPE_e        eventTypeIndex;
    LOG_EVENT_SUBTYPE_e     eventSubTypeIndex;
    LOG_EVENT_STATE_e       eventStateIndex;
    QString                 actualEventDateTime;
    QString                 actualEventDateTimeWithSec;
    QString                 expectedDateAndTime;
    quint8                  cameraNumber;
}LOG_EVENT_INFO_t;

class EventLogSearch : public BackGround
{
    Q_OBJECT
public:
    explicit EventLogSearch(QWidget *parent = 0);
    ~EventLogSearch();

    void createDefaultComponents();
    void getDevDateTime();
    void defaultState();
    void searchEventData();
    void fillRecordList();
    void clearSearchRecList(quint8 index);
    void searchCameraRecording(quint8 index);
    void processDeviceResponse(DevCommParam *param, QString deviceName);

    void takeLeftKeyAction();
    void takeRightKeyAction();
    void wheelEvent(QWheelEvent *);
    void showEvent(QShowEvent *event);

    //keyboard support added
    void navigationKeyPressed(QKeyEvent *event);
    void backTab_KeyPressed(QKeyEvent *event);
    void tabKeyPressed(QKeyEvent *event);
    void escKeyPressed(QKeyEvent *event);

    void updateDeviceList(void);
    void storeEventInfo();

signals:
    void sigPlaybackPlayBtnClick(PlaybackRecordData recData, QString devName,bool status);

public slots:
    void slotUpadateCurrentElement(int);
    void slotButtonClick(int);
    void slotInfoPageCnfgBtnClick(int);
    void slotEventScrollbarClick(int);

private:
    ApplController* applController;
    PayloadLib*     payloadLib;
    InfoPage*       infoPage;
    ProcessBar*     processBar;

    QString         currentDeviceName;
    DropDown*       deviceNameDropDownBox;

    CalendarTile*   startDateCalender;
    ClockSpinbox*   startTimeClockSpinbox;

    CalendarTile*   endDateCalender;
    ClockSpinbox*   endTimeClockSpinbox;

    DropDown*       eventListDropDownBox;
    ControlButton*  searchBtn;

    TableCell*      headingTitle[MAX_EVENT_LOG_DATA_COL];
    TextLabel*      headingTitleLabel[MAX_EVENT_LOG_DATA_COL];

    BgTile*         tableCellDataBg;

    TableCell*      indexNoTableCell[MAX_EVENT_LOG_DATA_ROW];
    TextLabel*      indexNoLabel[MAX_EVENT_LOG_DATA_ROW];

    TableCell*      dateAndTimeTabelCell[MAX_EVENT_LOG_DATA_ROW];
    TextLabel*      dateAndTimeLabel[MAX_EVENT_LOG_DATA_ROW];

    TableCell*      eventTypeTabelCell[MAX_EVENT_LOG_DATA_ROW];
    TextLabel*      eventTypeLabel[MAX_EVENT_LOG_DATA_ROW];

    TableCell*      eventTabelCell[MAX_EVENT_LOG_DATA_ROW];
    TextLabel*      eventLabel[MAX_EVENT_LOG_DATA_ROW];

    TableCell*      stateTabelCell[MAX_EVENT_LOG_DATA_ROW];
    TextLabel*      stateLabel[MAX_EVENT_LOG_DATA_ROW];

    TableCell*      sourceTabelCell[MAX_EVENT_LOG_DATA_ROW];
    TextLabel*      sourceLabel[MAX_EVENT_LOG_DATA_ROW];

    TableCell*      advanceDetailTabelCell[MAX_EVENT_LOG_DATA_ROW];
    TextLabel*      advanceDetailLabel[MAX_EVENT_LOG_DATA_ROW];

    TableCell*      playButtonTabelCell[MAX_EVENT_LOG_DATA_ROW];
    ControlButton*  playControlButton[MAX_EVENT_LOG_DATA_ROW];

    ScrollBar*      eventScrollbar;

    QStringList                 m_actualStartTimeList;
    QStringList                 m_actualStartTimeWithSecList;
    QStringList                 m_expectedEndTimeList;
    quint8                      m_cameraNumber[MAX_EVENT_LOG_DATA_ROW];
    RECORD_TYPE_e               m_currentEventNumber[MAX_EVENT_LOG_DATA_ROW];

    QStringList                 m_eventList;
    LiveEventParser*            m_liveEventParser;
    quint16                     m_totalEventIndex;
    quint16                     m_firstEventIndex, m_lastEventIndex;

    NavigationControl*          m_elementList[MAX_EVNT_LOG_CTRL];
    quint8                      m_currElement;

    quint8                      m_currentRequestIndex;
    DEV_TABLE_INFO_t            deviceInfo;
    QVector<LOG_EVENT_INFO_t>   m_logEvtInfo;
};

#endif // EVENTLOGSEARCH_H
