#include "LiveEventDetail.h"
#include <QKeyEvent>

#define LIVE_EVETEN_DETAIL_TOP_WIDTH    SCALE_WIDTH(290)
#define LIVE_EVETEN_DETAIL_TOP_HEIGHT   SCALE_HEIGHT(45)
#define LIVE_EVENT_DETAIL_LEFT_MARGIN   SCALE_WIDTH(15)
#define LIVE_EVENT_DETAIL_TOP_MARGIN    (SCALE_HEIGHT(30) + LIVE_EVETEN_DETAIL_TOP_HEIGHT)
#define INNER_MARGIN                    10

const QString tableHeaderString[] =
{
    "",
    "Date and Time",
    "Event Type",
    "Event",
    "State",
    "Source",
    "Advance Details"
};

const QString ConfigButtonString[] = {"Refresh", "Clear", "Clear All"};

const int tableCellWidth[] = {32, 225, 150, 190, 170, 250, 345};

LiveEventDetail::LiveEventDetail(int startX, int startY, QWidget *parent)
    : BackGround(startX, startY,
                 LIVE_EVENT_DETAIL_WIDTH, LIVE_EVENT_DETAIL_HEIGHT,
                 BACKGROUND_TYPE_1,
                 LIVE_EVENT_BUTTON,
                 parent,
                 LIVE_EVETEN_DETAIL_TOP_WIDTH, LIVE_EVETEN_DETAIL_TOP_HEIGHT,
                 "Event Notification"), m_totalPages(0)
{
    quint8 m_deviceCount;
    m_liveEventParser = new LiveEventParser();
    m_applController = ApplController::getInstance();
    m_applController->GetEnableDevList(m_deviceCount, m_enabledDeviceList);

    m_elementList[LIVE_EVENT_DETAIL_CLOSE_BUTTON] = m_mainCloseButton;
    m_mainCloseButton->setIndexInPage(LIVE_EVENT_DETAIL_CLOSE_BUTTON);
    connect(m_mainCloseButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    QMap<quint8, QString> deviceMapList;
    m_applController->GetDevNameDropdownMapList(deviceMapList);
    m_deviceNamePickList = new PickList(LIVE_EVENT_DETAIL_LEFT_MARGIN,
                                        LIVE_EVENT_DETAIL_TOP_MARGIN,
                                        (LIVE_EVENT_DETAIL_WIDTH - (2 * LIVE_EVENT_DETAIL_LEFT_MARGIN)),
                                        SCALE_HEIGHT(40),
                                        SCALE_WIDTH(150),
                                        SCALE_HEIGHT(30),
                                        "Device",
                                        deviceMapList,
                                        0,
                                        "Select Device",
                                        this,
                                        COMMON_LAYER,
                                        SCALE_WIDTH(10),
                                        DEVICENAME_PICK_LIST,
                                        true);
    connect(m_deviceNamePickList,
            SIGNAL(sigValueChanged(quint8,QString,int)),
            this,
            SLOT(slotDeviceChanged(quint8,QString,int)));
    connect(m_deviceNamePickList,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    m_elementList[DEVICENAME_PICK_LIST] = m_deviceNamePickList;

    //create tableheader tile having tabel heading fields
    m_headerTile = new BgTile(LIVE_EVENT_DETAIL_LEFT_MARGIN,
                              (m_deviceNamePickList->y() + m_deviceNamePickList->height() + SCALE_HEIGHT(5)),
                              (LIVE_EVENT_DETAIL_WIDTH - (2 * LIVE_EVENT_DETAIL_LEFT_MARGIN)),
                              SCALE_HEIGHT(30),
                              TOP_LAYER,
                              this);

    for(quint8 index = 0; index < MAX_FEILDS; index++)
    {
        quint16 offSet = (index == 0 ? (LIVE_EVENT_DETAIL_LEFT_MARGIN + SCALE_WIDTH(INNER_MARGIN))
                                     : (m_headerTableCell[index - 1]->width() - 1 + m_headerTableCell[index - 1]->x()));

        m_headerTableCell[index] = new TableCell(offSet,
                                                 (m_headerTile->y() + SCALE_HEIGHT(INNER_MARGIN)),
                                                 SCALE_WIDTH(tableCellWidth[index]),
                                                 SCALE_HEIGHT(30),
                                                 this,
                                                 true);

        m_tableCellHeadingText[index] = new TextLabel((m_headerTableCell[index]->x() + SCALE_WIDTH(10)),
                                                      (m_headerTableCell[index]->y() + m_headerTableCell[index]->height() / 2),
                                                      SCALE_FONT(SUB_HEADING_FONT_SIZE),
                                                      tableHeaderString[index],
                                                      this,
                                                      HIGHLITED_FONT_COLOR,
                                                      NORMAL_FONT_FAMILY,
                                                      ALIGN_START_X_CENTRE_Y,
                                                      0,
                                                      false,
                                                      SCALE_WIDTH(tableCellWidth[index]),
                                                      0, true, Qt::AlignVCenter, SCALE_WIDTH(8));
    }

    m_selectAllCheckBox = new OptionSelectButton((m_headerTableCell[0]->x() + SCALE_WIDTH(4)),
                                                 m_headerTableCell[0]->y(),
                                                 SCALE_WIDTH(30),
                                                 m_headerTableCell[0]->height(),
                                                 CHECK_BUTTON_INDEX,
                                                 this,
                                                 NO_LAYER,
                                                 "", "", 0,
                                                 SELECT_ALL_CHECKBOX,
                                                 false);
    connect(m_selectAllCheckBox,
            SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
            this,
            SLOT(slotCheckboxClicked(OPTION_STATE_TYPE_e,int)));
    connect(m_selectAllCheckBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    m_elementList[SELECT_ALL_CHECKBOX] = m_selectAllCheckBox;

    //create records table
    for(quint8 rowIndex = 0; rowIndex < MAX_DATA_ON_PAGE; rowIndex++)
    {
        m_dataTiles[rowIndex] = new BgTile(LIVE_EVENT_DETAIL_LEFT_MARGIN,
                                           (m_headerTile->y() + m_headerTile->height() + (rowIndex * SCALE_HEIGHT(30))),
                                           m_headerTile->width(),
                                           SCALE_HEIGHT(30),
                                           MIDDLE_LAYER,
                                           this);
        for(quint8 colIndex = 0; colIndex < MAX_FEILDS; colIndex++)
        {
            quint16 offSet = (colIndex == 0 ? (LIVE_EVENT_DETAIL_LEFT_MARGIN + SCALE_WIDTH(INNER_MARGIN))
                                            : (m_dataTableCell[rowIndex][colIndex - 1]->width() - 1 + m_dataTableCell[rowIndex][colIndex - 1]->x()));

            m_dataTableCell[rowIndex][colIndex] = new TableCell(offSet,
                                                                m_dataTiles[rowIndex]->y(),
                                                                SCALE_WIDTH(tableCellWidth[colIndex]),
                                                                SCALE_HEIGHT(30),
                                                                this);

            m_dataTextLabel[rowIndex][colIndex] = new TextLabel((m_dataTableCell[rowIndex][colIndex]->x() + SCALE_WIDTH(10)),
                                                                (m_dataTableCell[rowIndex][colIndex]->y() + (m_dataTableCell[rowIndex][colIndex]->height() / 2)),
                                                                NORMAL_FONT_SIZE,
                                                                "",
                                                                this,
                                                                NORMAL_FONT_COLOR,
                                                                NORMAL_FONT_FAMILY,
                                                                ALIGN_START_X_CENTRE_Y,
                                                                0,
                                                                false,
                                                                SCALE_WIDTH(tableCellWidth[colIndex]),
                                                                0, true, Qt::AlignVCenter, SCALE_WIDTH(8));
        }

        m_selectSingleRecordCheckBox[rowIndex] = new OptionSelectButton((m_dataTableCell[rowIndex][0]->x() + SCALE_WIDTH(4)),
                                                                        m_dataTableCell[rowIndex][0]->y(),
                                                                        SCALE_WIDTH(30),
                                                                        m_dataTableCell[rowIndex][0]->height(),
                                                                        CHECK_BUTTON_INDEX,
                                                                        this, NO_LAYER,
                                                                        "", "", 0,
                                                                        (rowIndex + 1 + SELECT_ALL_CHECKBOX),
                                                                        false);
        connect(m_selectSingleRecordCheckBox[rowIndex],
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotCheckboxClicked(OPTION_STATE_TYPE_e,int)));
        connect(m_selectSingleRecordCheckBox[rowIndex],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        m_elementList[rowIndex + 1 + SELECT_ALL_CHECKBOX] = m_selectSingleRecordCheckBox[rowIndex];
        m_selectSingleRecordCheckBox[rowIndex]->setVisible(false);
    }

    //create bottom tile having navigation buttons and page number
    m_bottomTile = new BgTile(LIVE_EVENT_DETAIL_LEFT_MARGIN,
                              (this->height() - SCALE_HEIGHT(100)),
                              (LIVE_EVENT_DETAIL_WIDTH - (2 * LIVE_EVENT_DETAIL_LEFT_MARGIN)),
                              SCALE_HEIGHT(30),
                              BOTTOM_LAYER,
                              this);

    m_firstPage = new ControlButton(FIRSTPAGE_BUTTON_INDEX,
                                    SCALE_WIDTH(581),
                                    m_bottomTile->y(),
                                    SCALE_WIDTH(40), SCALE_HEIGHT(40),
                                    this,
                                    NO_LAYER,
                                    0, "",
                                    false,
                                    FIRST_PAGE_BUTTON);
    connect(m_firstPage,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotNavigationButtonClicked(int)));
    connect(m_firstPage,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    m_elementList[FIRST_PAGE_BUTTON] = m_firstPage;

    m_previousPage = new ControlButton(PREVIOUS_BUTTON_INDEX,
                                       (m_firstPage->x() + m_firstPage->width() + SCALE_WIDTH(10)),
                                       m_bottomTile->y(),
                                       SCALE_WIDTH(40), SCALE_HEIGHT(40),
                                       this,
                                       NO_LAYER,
                                       0, "",
                                       false,
                                       PREVIOUS_PAGE_BUTTON);
    connect(m_previousPage,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotNavigationButtonClicked(int)));
    connect(m_previousPage,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    m_elementList[PREVIOUS_PAGE_BUTTON] = m_previousPage;

    m_pageNumber = new ReadOnlyElement((((m_bottomTile->width() - SCALE_WIDTH(90)) / 2) + m_bottomTile->x()),
                                       (((m_bottomTile->height() - SCALE_HEIGHT(30)) / 2) + m_bottomTile->y()),
                                       SCALE_WIDTH(90), SCALE_HEIGHT(30), SCALE_WIDTH(90), SCALE_HEIGHT(30), "",
                                       this, NO_LAYER);

    m_nextPage = new ControlButton(NEXT_BUTTON_INDEX,
                                   (m_pageNumber->x() + m_pageNumber->width() + SCALE_WIDTH(10)),
                                   m_bottomTile->y(),
                                   SCALE_WIDTH(40), SCALE_HEIGHT(40),
                                   this,
                                   NO_LAYER,
                                   0, "",
                                   false,
                                   NEXT_PAGE_BUTTON);
    connect(m_nextPage,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotNavigationButtonClicked(int)));
    connect(m_nextPage,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    m_elementList[NEXT_PAGE_BUTTON] = m_nextPage;

    m_lastPage = new ControlButton(LAST_BUTTON_INDEX,
                                   (m_nextPage->x() + m_nextPage->width() + SCALE_WIDTH(10)),
                                   m_bottomTile->y(),
                                   SCALE_WIDTH(40), SCALE_HEIGHT(40), this,
                                   NO_LAYER,
                                   0, "",
                                   false,
                                   LAST_PAGE_BUTTON);
    connect(m_lastPage,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotNavigationButtonClicked(int)));
    connect(m_lastPage,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    m_elementList[LAST_PAGE_BUTTON] = m_lastPage;

    //create config button for clear, clearall, and refresh
    m_clearButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                   (LIVE_EVENT_DETAIL_LEFT_MARGIN + (LIVE_EVENT_DETAIL_WIDTH / 2)),
                                   (LIVE_EVENT_DETAIL_HEIGHT + LIVE_EVETEN_DETAIL_TOP_HEIGHT - SCALE_HEIGHT(30)),
                                   ConfigButtonString[1],
                                   this,
                                   CLEAR_BUTTON,
                                   false);
    connect(m_clearButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotConfigButtonClicked(int)));
    connect(m_clearButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    m_elementList[CLEAR_BUTTON] = m_clearButton;

    m_refreshButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                     (m_clearButton->x() - (m_clearButton->width() / 2)),
                                     (LIVE_EVENT_DETAIL_HEIGHT + LIVE_EVETEN_DETAIL_TOP_HEIGHT - SCALE_HEIGHT(30)),
                                     ConfigButtonString[0],
                                     this,
                                     REFRESH_BUTTON,
                                     true);
    connect(m_refreshButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotConfigButtonClicked(int)));
    connect(m_refreshButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    m_elementList[REFRESH_BUTTON] = m_refreshButton;

    m_clearAllButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                      (m_clearButton->x() + (m_clearButton->width() / 2) + m_clearButton->width()),
                                      (LIVE_EVENT_DETAIL_HEIGHT + LIVE_EVETEN_DETAIL_TOP_HEIGHT - SCALE_HEIGHT(30)),
                                      ConfigButtonString[2],
                                      this,
                                      CLEAR_ALL_BUTTON,
                                      false);
    connect(m_clearAllButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotConfigButtonClicked(int)));
    connect(m_clearAllButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    m_elementList[CLEAR_ALL_BUTTON] = m_clearAllButton;

    m_currentPageIndex = 0;
    m_currentDeviceIndex = 0;
    m_currentElement = DEVICENAME_PICK_LIST;
}

LiveEventDetail::~LiveEventDetail()
{
    disconnect(m_mainCloseButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));

    //delete picklist
    disconnect(m_deviceNamePickList,
               SIGNAL(sigValueChanged(quint8,QString,int)),
               this,
               SLOT(slotDeviceChanged(quint8,QString,int)));
    disconnect(m_deviceNamePickList,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_deviceNamePickList;

    //delete tableheader tile having tabel heading fields
    delete m_headerTile;
    for(int index = 0; index < MAX_FEILDS; index++)
    {
        delete m_headerTableCell[index];
        delete m_tableCellHeadingText[index];
    }

    disconnect(m_selectAllCheckBox,
               SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
               this,
               SLOT(slotCheckboxClicked(OPTION_STATE_TYPE_e,int)));
    disconnect(m_selectAllCheckBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_selectAllCheckBox;

    //delete records table
    for(quint8 rowIndex = 0; rowIndex < MAX_DATA_ON_PAGE; rowIndex++)
    {
        delete m_dataTiles[rowIndex];
        for(quint8 colIndex = 0; colIndex < MAX_FEILDS; colIndex++)
        {
            delete m_dataTableCell[rowIndex][colIndex];
            delete m_dataTextLabel[rowIndex][colIndex];
        }

        disconnect(m_selectSingleRecordCheckBox[rowIndex],
                   SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                   this,
                   SLOT(slotCheckboxClicked(OPTION_STATE_TYPE_e,int)));
        disconnect(m_selectSingleRecordCheckBox[rowIndex],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        delete m_selectSingleRecordCheckBox[rowIndex];
    }

    //delete bottom tile having navigation buttons and page number
    delete m_bottomTile;
    disconnect(m_firstPage,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotNavigationButtonClicked(int)));
    disconnect(m_firstPage,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_firstPage;

    disconnect(m_previousPage,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotNavigationButtonClicked(int)));
    disconnect(m_previousPage,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_previousPage;

    delete m_pageNumber;
    disconnect(m_nextPage,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotNavigationButtonClicked(int)));
    disconnect(m_nextPage,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_nextPage;

    disconnect(m_lastPage,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotNavigationButtonClicked(int)));
    disconnect(m_lastPage,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_lastPage;

    //delete config button for clear, clearall, and refresh
    disconnect(m_clearButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotConfigButtonClicked(int)));
    disconnect(m_clearButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_clearButton;

    disconnect(m_refreshButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotConfigButtonClicked(int)));
    disconnect(m_refreshButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_refreshButton;

    disconnect(m_clearAllButton,
               SIGNAL(sigButtonClick(int)),
               this,
               SLOT(slotConfigButtonClicked(int)));
    disconnect(m_clearAllButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete m_clearAllButton;

    DELETE_OBJ(m_liveEventParser);
 }

void LiveEventDetail::changeDevice(QString deviceName)
{
    int deviceIndex = m_enabledDeviceList.indexOf(deviceName);
    m_currentDeviceIndex = deviceIndex;
    m_deviceNamePickList->changeValue(deviceIndex);
    if(getEventList(deviceName))
    {
        parseEventList();
    }
    else
    {
        m_pageNumber->changeValue("");
        m_clearAllButton->setIsEnabled(false);
        m_selectAllCheckBox->setIsEnabled(false);
        m_selectAllCheckBox->changeState(OFF_STATE);
        clearRecords();
    }
    updateNavigationControlStatus();
}

bool LiveEventDetail::getEventList(QString deviceName)
{
    m_eventList.clear();
    m_totalPages = 0;
    m_currentPageIndex = 0;
    bool status = false;
    status = m_applController->GetDeviceEventList(deviceName, m_eventList);
    if(status == true)
    {
        status = (m_eventList.length() > 0);
    }
    return status;
}

void LiveEventDetail::parseEventList()
{
    m_selectAllCheckBox->setIsEnabled(true);
    m_selectAllCheckBox->changeState(OFF_STATE);
    m_clearAllButton->setIsEnabled(true);

    m_totalPages = (m_eventList.length() / MAX_DATA_ON_PAGE);
    if(m_eventList.length() % MAX_DATA_ON_PAGE != 0)
    {
        m_totalPages += 1;
    }
    fillRecords(0);
}

void LiveEventDetail::fillRecords(int pageIndex)
{
    //clear previous records
    clearRecords();

    m_currentPageIndex = pageIndex;
    m_pageNumber->changeValue("Page" + QString(" %1").arg(m_currentPageIndex + 1));
    int startingIndex = m_currentPageIndex * MAX_DATA_ON_PAGE;

    //fill new records
    for(quint8 index = startingIndex; (index < (startingIndex + MAX_DATA_ON_PAGE) && (index < m_eventList.length())); index++)
    {
        m_liveEventParser->setEventString(m_eventList.at(index));
        m_dataTextLabel[index % MAX_DATA_ON_PAGE][EVENT_DATE_AND_TIME]->changeText(m_liveEventParser->getDateAndTime());
        m_dataTextLabel[index % MAX_DATA_ON_PAGE][EVENT_TYPE]->changeText(m_liveEventParser->getEventType());
        m_dataTextLabel[index % MAX_DATA_ON_PAGE][EVENT_SUB_TYPE]->changeText(m_liveEventParser->getEventSubType());
        m_dataTextLabel[index % MAX_DATA_ON_PAGE][EVENT_STATE]->changeText(m_liveEventParser->getEventState());
        m_dataTextLabel[index % MAX_DATA_ON_PAGE][EVENT_SOURCE]->changeText(m_liveEventParser->getEventSource());
        m_dataTextLabel[index % MAX_DATA_ON_PAGE][EVENT_ADVANCE_DETAIL]->changeText(m_liveEventParser->getEventAdvanceDetail());
        m_selectSingleRecordCheckBox[index % MAX_DATA_ON_PAGE]->setVisible(true);
        m_selectSingleRecordCheckBox[index % MAX_DATA_ON_PAGE]->setIsEnabled(true);
    }
}

void LiveEventDetail::clearRecords()
{
    for(quint8 index = 0; index < MAX_DATA_ON_PAGE; index++)
    {
        m_selectSingleRecordCheckBox[index]->setVisible(false);
        m_selectSingleRecordCheckBox[index]->setIsEnabled(false);
        m_selectSingleRecordCheckBox[index]->changeState(OFF_STATE);

        m_dataTextLabel[index][EVENT_DATE_AND_TIME]->changeText("");
        m_dataTextLabel[index][EVENT_TYPE]->changeText("");
        m_dataTextLabel[index][EVENT_SUB_TYPE]->changeText("");
        m_dataTextLabel[index][EVENT_STATE]->changeText("");
        m_dataTextLabel[index][EVENT_SOURCE]->changeText("");
        m_dataTextLabel[index][EVENT_ADVANCE_DETAIL]->changeText("");
    }

    m_selectAllCheckBox->changeState(OFF_STATE);
    m_clearButton->setIsEnabled(false);
}

void LiveEventDetail::updateNavigationControlStatus()
{
    bool isButtonDisable[4];
    bool buttonFound = false;
    for(quint8 index = 0; index < 4; index++)
    {
        isButtonDisable[index] = true;
    }

    if(m_totalPages > 1)
    {
        isButtonDisable[0] = (m_currentPageIndex == 0);
        isButtonDisable[1] = (m_currentPageIndex <= 0);
        isButtonDisable[2] = (m_currentPageIndex >= (m_totalPages - 1));
        isButtonDisable[3] = (m_currentPageIndex == (m_totalPages - 1));
    }

    for(quint8 index = 0; index < 4; index++)
    {
        quint8 buttonIndex = (index + FIRST_PAGE_BUTTON);
        quint8 startingButtonIndex = 0, endingButtonIndex = 0;
        if((buttonIndex == m_currentElement) && (isButtonDisable[index]))
        {
            switch(buttonIndex)
            {
                case FIRST_PAGE_BUTTON:
                    startingButtonIndex = 1;
                    endingButtonIndex = 4;
                    break;

                case PREVIOUS_PAGE_BUTTON:
                    startingButtonIndex = 2;
                    endingButtonIndex = 4;
                    break;

                case NEXT_PAGE_BUTTON:
                    startingButtonIndex = 0;
                    endingButtonIndex = 2;
                    break;

                case LAST_PAGE_BUTTON:
                    startingButtonIndex = 0;
                    endingButtonIndex = 3;
                    break;
            }

            for(quint8 button = startingButtonIndex; button < endingButtonIndex; button++)
            {
                if(!isButtonDisable[button])
                {
                    m_currentElement = (button + FIRST_PAGE_BUTTON);
                    buttonFound = true;
                    if((buttonIndex == LAST_PAGE_BUTTON) || (buttonIndex == PREVIOUS_PAGE_BUTTON))
                    {
                        break;
                    }
                }
            }
            break;
        }
    }

    m_firstPage->setIsEnabled(!isButtonDisable[0]);
    m_previousPage->setIsEnabled(!isButtonDisable[1]);
    m_nextPage->setIsEnabled(!isButtonDisable[2]);
    m_lastPage->setIsEnabled(!isButtonDisable[3]);

    if(!buttonFound)
    {
        m_currentElement = REFRESH_BUTTON;
    }
    m_elementList[m_currentElement]->forceActiveFocus();
}

bool LiveEventDetail::deleteSelectedEvent()
{
    QStringList listForDeletion;

    for(quint8 index = 0; index < MAX_DATA_ON_PAGE; index++)
    {
        if(m_selectSingleRecordCheckBox[index]->getCurrentState())
        {
            listForDeletion.append(m_eventList.at(index + (m_currentPageIndex * MAX_DATA_ON_PAGE)));
        }
    }

    return deleteEvents(listForDeletion);
}

bool LiveEventDetail::deleteEvents(QStringList listForDeletion)
{
    return m_applController->DeleteDeviceEventList(m_enabledDeviceList.at(m_currentDeviceIndex), listForDeletion);
}

void LiveEventDetail::selectDeselectAllRecords(OPTION_STATE_TYPE_e state)
{
    for(quint8 index = 0; index < MAX_DATA_ON_PAGE; index++)
    {
        if(m_selectSingleRecordCheckBox[index]->isVisible())
        {
            m_selectSingleRecordCheckBox[index]->changeState(state);
        }
    }
}

bool LiveEventDetail::isAnyButtonChecked()
{
    for(quint8 index = 0; index < MAX_DATA_ON_PAGE; index++)
    {
        if((m_selectSingleRecordCheckBox[index]->isVisible()) && (m_selectSingleRecordCheckBox[index]->getCurrentState()))
        {
            return true;
        }
    }

    return false;
}

bool LiveEventDetail::isAllButtonChecked()
{
    for(quint8 index = 0; index < MAX_DATA_ON_PAGE; index++)
    {
        if((m_selectSingleRecordCheckBox[index]->isVisible()) && (!m_selectSingleRecordCheckBox[index]->getCurrentState()))
        {
            return false;
        }
    }
    return true;
}

void LiveEventDetail::forceActiveFocus()
{
    m_elementList[m_currentElement]->forceActiveFocus();
}

void LiveEventDetail::takeLeftKeyAction()
{
    do
    {
        m_currentElement = (m_currentElement - 1 + MAX_LIVE_EVENT_DETAIL_ELEMENT) % MAX_LIVE_EVENT_DETAIL_ELEMENT;
    }while(!m_elementList[m_currentElement]->getIsEnabled());

    m_elementList[m_currentElement]->forceActiveFocus();
}

void LiveEventDetail::takeRightKeyAction()
{
    do
    {
        m_currentElement = (m_currentElement + 1) % MAX_LIVE_EVENT_DETAIL_ELEMENT;
    }while(!m_elementList[m_currentElement]->getIsEnabled());

    m_elementList[m_currentElement]->forceActiveFocus();
}

void LiveEventDetail::showEvent (QShowEvent *event)
{
    QWidget::showEvent (event);
    m_elementList[m_currentElement]->forceActiveFocus ();
}

void LiveEventDetail::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void LiveEventDetail::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void LiveEventDetail::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void LiveEventDetail::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    m_currentElement = LIVE_EVENT_DETAIL_CLOSE_BUTTON;

    if((m_currentElement < MAX_LIVE_EVENT_DETAIL_ELEMENT) && (IS_VALID_OBJ(m_elementList[m_currentElement])))
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

void LiveEventDetail::slotDeviceChanged(quint8, QString value, int)
{
    changeDevice(m_applController->GetRealDeviceName(value));
}

void LiveEventDetail::slotNavigationButtonClicked(int index)
{
    switch(index)
    {
        case FIRST_PAGE_BUTTON:
            fillRecords(0);
            break;

        case PREVIOUS_PAGE_BUTTON:
            fillRecords(m_currentPageIndex - 1);
            break;

        case NEXT_PAGE_BUTTON:
            fillRecords(m_currentPageIndex + 1);
            break;

        case LAST_PAGE_BUTTON:
            fillRecords(m_totalPages - 1);
            break;
    }
    updateNavigationControlStatus();
}

void LiveEventDetail::slotConfigButtonClicked(int index)
{
    bool status = false;
    switch(index)
    {
        case REFRESH_BUTTON:
            status = true;
            break;
        case CLEAR_BUTTON:
            status = deleteSelectedEvent();
            break;
        case CLEAR_ALL_BUTTON:
            status = deleteEvents(m_eventList);
            break;
    }

    if(status)
    {
        changeDevice(m_enabledDeviceList.at(m_currentDeviceIndex));
        if(index != REFRESH_BUTTON)
        {
            QStringList list;
            bool isEmptyList = true;
            for(quint8 index = 0; index < m_enabledDeviceList.length(); index++)
            {
                list.clear();
                m_applController->GetDeviceEventList(m_enabledDeviceList.at(index), list);
                if(list.length() != 0)
                {
                    isEmptyList = false;
                    break;
                }
            }

            if(isEmptyList)
            {
                emit sigNotifyStatusIcon(LIVE_EVENT_BUTTON, false);
            }

            list.clear();
        }
    }
}

void LiveEventDetail::slotCheckboxClicked(OPTION_STATE_TYPE_e currentState, int index)
{
    if(index == SELECT_ALL_CHECKBOX)
    {
        selectDeselectAllRecords(currentState);
    }
    else
    {
        m_selectAllCheckBox->changeState((OPTION_STATE_TYPE_e)isAllButtonChecked());
    }
    if(isAnyButtonChecked())
    {
        m_clearButton->setIsEnabled(true);
    }
    else
    {
        m_clearButton->setIsEnabled(false);
    }
}

void LiveEventDetail::slotUpdateCurrentElement(int index)
{
    m_currentElement = index;
}

void LiveEventDetail::functionKeyPressed(QKeyEvent *event)
{
    switch(event->key())
    {
        default:
            break;

        case Qt::Key_F5:
            slotConfigButtonClicked(REFRESH_BUTTON);
            break;
    }
}

void LiveEventDetail::updateDeviceList(void)
{
    quint8                  m_deviceCount;
    quint8                  selectedIndex = 0;
    QMap<quint8, QString>   deviceMapList;
    m_applController->GetDevNameDropdownMapList(deviceMapList);

    /* Delete and remove pick list if it is displaying */
    m_deviceNamePickList->deletePicklistLoader();

    /* Update the list of enabled devices */
    m_applController->GetEnableDevList(m_deviceCount, m_enabledDeviceList);

    /* Check if selected device found in new updated list or not. It is possible that index of that device name may changed.
     * Hence update device list with current device index */
    for (quint8 deviceIndex = 0; deviceIndex < deviceMapList.count(); deviceIndex++)
    {
        if (m_deviceNamePickList->getCurrentPickStr() == deviceMapList.value(deviceIndex))
        {
            selectedIndex = deviceIndex;
            break;
        }
    }

    /* If selected device is local device then it will update the device name  and will get the list for the local device */
    m_deviceNamePickList->changeOptionList(deviceMapList, selectedIndex, false);
    slotDeviceChanged(selectedIndex, deviceMapList.value(selectedIndex), DEVICENAME_PICK_LIST);
}
