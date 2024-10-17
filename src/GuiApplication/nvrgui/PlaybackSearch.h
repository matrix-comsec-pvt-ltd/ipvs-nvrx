#ifndef PLAYBACKSEARCH_H
#define PLAYBACKSEARCH_H


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

#include "Controls/PlaybackControl/BackupSingleRecord.h"
#include "Controls/PlaybackControl/BackupRecords.h"
#include "Controls/TextLabel.h"
#include "Controls/InfoPage.h"
#include "Controls/ProcessBar.h"
// to know manual back up device exist or not
#include "UsbControl.h"

#define MAX_REC_IN_ONE_PAGE             10
#define MAX_REC_ALLOWED                 (MAX_REC_IN_ONE_PAGE * MAX_REC_IN_ONE_PAGE)
#define MAX_PB_SEARCH_TABLE_COL         7
#define MAX_PB_SEARCH_TABLE_ROW         10

#define MAX_REC_TO_BACKUP               10
#define MAX_REC_PAGES                   10

typedef enum
{
    PB_SEARCH_CLOSE_BTN,
    PB_SEARCH_DEVICE_SPINBOX,
    PB_SEARCH_REC_DRIVE_DROPBOX,
    PB_SEARCH_START_DATE,
    PB_SEARCH_START_TIME,
    PB_SEARCH_END_DATE,
    PB_SEARCH_END_TIME,
    PB_SEARCH_REC_TYPE_SPINBOX,
    PB_SEARCH_CHANNEL_SPINBOX,
    PB_SEARCH_SEARCH_BTN,

    PB_SEARCH_SEL_ALL,

    PB_SEARCH_SEL_1ST,          //11
    PB_SEARCH_PLAY_1ST,         //12

    PB_SEARCH_SEL_10TH = PB_SEARCH_SEL_1ST + 18,        //29
    PB_SEARCH_PLAY_10TH = PB_SEARCH_PLAY_1ST + 18,      //30

    PB_SEARCH_FIRST_PAGE,               //31
    PB_SEARCH_PREV_PAGE,
    PB_SEARCH_NEXT_PAGE,
    PB_SEARCH_LAST_PAGE,

    PB_SEARCH_BACKUP_BTN,           //35

    MAX_PB_SEARCH_ELEMENTS          //36
}PB_SEARCH_ELEMENTS_e;

typedef struct _TEMP_PLAYBACK_REC_DATA_t
{
    quint8 tempType;
    quint8 tempChannel;
    quint8 tempSource;
    quint8 currentPage;
    QDateTime tempStartTime;
    QDateTime tempEndTime;
    _TEMP_PLAYBACK_REC_DATA_t() : tempType(0), tempChannel(0), tempSource(0), currentPage(0)
    {

    }
}TEMP_PLAYBACK_REC_DATA_t;
class PlaybackSearch : public BackGround
{
    Q_OBJECT
private:
    bool isBackupReqSent;
    bool recSelect[MAX_REC_ALLOWED];
    bool isQuickBackupon;
    quint8 backUpSingleRecIndex;
    QString m_currDevName;
    quint8 totalCam;
    quint8 totalRec;
    quint8 totalPage;
    quint8 currPage;
    QStringList channelList;
    quint8 totalTick;
//    OPTION_STATE_TYPE_e selAllCheckboxSatus[MAX_REC_PAGES];
    quint8 selAllOptionOfWhichPage;

    ApplController *applController;
    PayloadLib *payloadLib;
    InfoPage *infoPage;
    ProcessBar *processBar;

    DropDown *devNameDropDownBox, *recTypeDropDownBox, *channelDropDownBox;
    CalendarTile *startDateCalender, *endDateCalender;
    ClockSpinbox *startTimeSpinbox, *endTimeSpinbox;
    ControlButton *searchBtn;

    BgTile *tableCellHeaderBg;
    TableCell *headingTableCell[MAX_PB_SEARCH_TABLE_COL];
    TextLabel*headingTextLabel[MAX_PB_SEARCH_TABLE_COL];
    OptionSelectButton *selAllCheckbox;

    OptionSelectButton *recSelCheckboxes[MAX_PB_SEARCH_TABLE_ROW];
    ControlButton *playBtns[MAX_PB_SEARCH_TABLE_ROW];

    BgTile *tableCellDataBg;
    TableCell *recDataTableCell[MAX_PB_SEARCH_TABLE_ROW][MAX_PB_SEARCH_TABLE_COL];
    TextLabel *recDataTextLabel[MAX_PB_SEARCH_TABLE_ROW][MAX_PB_SEARCH_TABLE_COL - 1];

    ReadOnlyElement *pageNoReadOnly;
    ControlButton *firstPageBtn;
    ControlButton *prevPageBtn;
    ControlButton *nextPageBtn;
    ControlButton *lastPageBtn;
    CnfgButton *backupBtn;

    BackupSingleRecord *backupSingleRecord;
    BackupRecords *backupRecords;

    NavigationControl* m_elementList[MAX_PB_SEARCH_ELEMENTS];
    int m_currElement;

    PlaybackRecordData playbackRecData[MAX_REC_ALLOWED];
    DropDown * m_recDriveListDropdown;

    QStringList              m_cameraList;
    quint8                   m_cameraIndex[MAX_CAMERAS];
    TEMP_PLAYBACK_REC_DATA_t tempData;
    DDMMYY_PARAM_t PreviousDataCalender;
    DDMMYY_PARAM_t endDataCalender;

public:
    explicit PlaybackSearch(QWidget *parent =0);
    ~PlaybackSearch();

    void createDefaultComponent ();
    void getDevDateTime();
    void processDeviceResponse(DevCommParam *param, QString deviceName);
    void showTableData();
    void makeDatetimeInTableFormat(QString &dateTimeStr);

    void showEvent (QShowEvent * event);
    void takeLeftKeyAction();
    void takeRightKeyAction();
    void sendBackupCmd(QString startTime,
                       QString endTime);
    void sendMultipleRecBackupCmd();
    void fillMultipleRecBackupCmd();
    void sendStopBackupCmd();
    void resetOptionSelSateOnPageChange();
    void manualBkpSysEvtAction(QString devName, LOG_EVENT_STATE_e evtState);
    void updateManualBkpStatusEvtAction(QString devName, quint8 percent);
    void createFileCopystatusPage();
    void deleteBackupPage();
    void clearTableData(quint8 recIndex = 0);
    void setQuickBackupFlag(bool flag);

    void getManualBackupLocation();
    void showPrevTableData(PlaybackRecordData* prevData, quint8 totalrec, TEMP_PLAYBACK_REC_DATA_t *prevRec);
    quint8 getCurrPage();
    void restoreSerchCriteria(QString devName);
    void updateDeviceList(void);

    //keyboard functions
    void navigationKeyPressed(QKeyEvent *event);
    void backTab_KeyPressed(QKeyEvent *event);
    void tabKeyPressed(QKeyEvent *event);
    void escKeyPressed(QKeyEvent *event);

signals:
    void sigPlaybackPlayBtnClick(PlaybackRecordData recData, QString devName, bool status);
    void sigPreviousRecords(PlaybackRecordData* prevData,quint8 totalRecord, TEMP_PLAYBACK_REC_DATA_t* tempData);
    //void sigPreviousRecords( TEMP_PLAYBACK_REC_DATA_t* tempData,quint8 totalRecord);

public slots:
    void slotSpinboxValueChanged(QString str,quint32 index);
    void slotControlBtnclick(int index);
    void slotInfoPageCnfgBtnClick(int);
    void slotAllOptionSelBtnClick(OPTION_STATE_TYPE_e state, int index);
    void slotUpadateCurrentElement (int index);
    void slotDateChanged();

    void slotBackupBtnClicked(int index);
    void slotBackUpSingleRecHandling(int index);
    void slotBackUpRecsHandling(int index);
    void slotFileCopyStatusHandling(int index);

private:
    bool isCameraRightsAvailable(quint8 camIndex);
    void enableDisableControls(bool status);
};

#endif // PLAYBACKSEARCH_H
