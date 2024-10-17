#ifndef LIVEEVENTDETAIL_H
#define LIVEEVENTDETAIL_H

#include "Controls/BackGround.h"
#include "Controls/PickList.h"
#include "ApplController.h"
#include "LiveEventParser.h"
#include "Controls/TableCell.h"
#include "Controls/Bgtile.h"
#include "Controls/Heading.h"
#include "Controls/TextLabel.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/ReadOnlyElement.h"
#include "Controls/ControlButton.h"
#include "Controls/CnfgButton.h"

#define LIVE_EVENT_DETAIL_WIDTH     SCALE_WIDTH(1412)
#define LIVE_EVENT_DETAIL_HEIGHT    SCALE_HEIGHT(515)
#define MAX_DATA_ON_PAGE            10

typedef enum
{
    LIVE_EVENT_DETAIL_CLOSE_BUTTON = 0,
    DEVICENAME_PICK_LIST,
    SELECT_ALL_CHECKBOX,
    FIRST_PAGE_BUTTON = 13,
    PREVIOUS_PAGE_BUTTON,
    NEXT_PAGE_BUTTON,
    LAST_PAGE_BUTTON,
    REFRESH_BUTTON,
    CLEAR_BUTTON,
    CLEAR_ALL_BUTTON,
    MAX_LIVE_EVENT_DETAIL_ELEMENT
}LIVE_EVENT_DETAIL_ELEMENT_e;

typedef enum
{
    INDEX,
    EVENT_DATE_AND_TIME,
    EVENT_TYPE,
    EVENT_SUB_TYPE,
    EVENT_STATE,
    EVENT_SOURCE,
    EVENT_ADVANCE_DETAIL,
    MAX_FEILDS
}LIVE_EVENT_FIELD_TYPE_e;

class LiveEventDetail : public BackGround
{
    Q_OBJECT
private:
    int m_currentDeviceIndex;
    PickList* m_deviceNamePickList;
    ApplController* m_applController;
    LiveEventParser* m_liveEventParser;
    BgTile* m_headerTile;
    BgTile* m_bottomTile;
    BgTile* m_dataTiles[MAX_DATA_ON_PAGE];
    TableCell* m_headerTableCell[MAX_FEILDS];
    TextLabel* m_tableCellHeadingText[MAX_FEILDS];
    TextLabel* m_dataTextLabel[MAX_DATA_ON_PAGE][MAX_FEILDS];
    TableCell* m_dataTableCell[MAX_DATA_ON_PAGE][MAX_FEILDS];
    OptionSelectButton* m_selectAllCheckBox;
    OptionSelectButton* m_selectSingleRecordCheckBox[MAX_DATA_ON_PAGE];
    ReadOnlyElement* m_pageNumber;
    ControlButton* m_firstPage;
    ControlButton* m_previousPage;
    ControlButton* m_nextPage;
    ControlButton* m_lastPage;
    CnfgButton* m_refreshButton;
    CnfgButton* m_clearButton;
    CnfgButton* m_clearAllButton;

    QStringList m_enabledDeviceList;
    QStringList m_eventList;
    int m_currentPageIndex, m_totalPages;

    int m_currentElement;
    NavigationControl* m_elementList[MAX_LIVE_EVENT_DETAIL_ELEMENT];

public:
    LiveEventDetail(int startX, int startY, QWidget *parent = 0);
    ~LiveEventDetail();

    void changeDevice(QString deviceName);
    bool getEventList(QString deviceName);
    void parseEventList();
    void fillRecords(int pageIndex);
    void clearRecords();
    void updateNavigationControlStatus();
    bool deleteSelectedEvent();
    bool deleteEvents(QStringList listForDeletion);
    void selectDeselectAllRecords(OPTION_STATE_TYPE_e state);
    bool isAnyButtonChecked();
    bool isAllButtonChecked();
    void forceActiveFocus();

    void takeLeftKeyAction();
    void takeRightKeyAction();

    void showEvent (QShowEvent *event);
    void updateDeviceList(void);

    //keyboard support added
    void navigationKeyPressed(QKeyEvent *event);
    void escKeyPressed(QKeyEvent *event);
    void backTab_KeyPressed(QKeyEvent *event);
    void tabKeyPressed(QKeyEvent *event);
    void functionKeyPressed(QKeyEvent *event);

signals:
    void sigNotifyStatusIcon(TOOLBAR_BUTTON_TYPE_e index, bool state);

public slots:
    void slotDeviceChanged(quint8 index, QString value, int indexInPage);
    void slotNavigationButtonClicked(int index);
    void slotConfigButtonClicked(int index);
    void slotCheckboxClicked(OPTION_STATE_TYPE_e currentState, int index);
    void slotUpdateCurrentElement(int index);
};

#endif // LIVEEVENTDETAIL_H
