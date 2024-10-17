#include "PlaybackSearch.h"
#include <math.h>
#include "Layout/Layout.h"
#include <QKeyEvent>
#include "Controls/MessageBanner.h"
#include "ValidationMessage.h"

#define MAX_MAN_BACKUP_END_FEILDS           4
#define BACKUP_LOCATION_FEILD               4

#define PB_SEARCH_HEADING                   "Playback Search"

#define INVALID_FEILD 255

typedef enum
{
    REC_TYPE_ALL,
    REC_TYPE_MANUAL,
    REC_TYPE_ALARM,
    REC_TYPE_SCHEDULE,
    REC_TYPE_COSEC,

    MAX_REC_TYPE
}REC_TYPE_e;

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
    FIELD_STORAGE,

    MAX_FIELD_NO
}CNFG_FIELD_NO_e;

const static QString labelStr[MAX_PB_SEARCH_ELEMENTS] =
{
    "",
    "Devices",
    "Source",
    "Start Date",
    "",
    "End Date",
    "",
    "Type",
    "Camera",
    "Search"
};

const static QString tableCellHeadingStr[MAX_PB_SEARCH_TABLE_COL] =
{
    "Sr.",
    "Start Time",
    "End Time",
    "Camera",
    "Type",
    "Source",
    "Play"
};

static const QMap<quint8, QString> recTypeStrMapList =
{
    {0, "All"},
    {1, "Manual"},
    {2, "Alarm"},
    {3, "Schedule"},
    #if !defined(OEM_JCI)
    {4, "COSEC"},
    #endif
};

static const QStringList recDriveList = QStringList()
        << "All" << "Local Drive" <<  "Network Drive 1" << "Network Drive 2";

static const QStringList recDriveListforTable = QStringList()
        << "Local Drive" <<  "Network Drive 1" << "Network Drive 2";

const static quint8 recTypeValue[5] = {15, 1, 2, 4, 8};

const static QString recTypeStr[16] =
{
    "All",
    "Manual",
    "Alarm",
    "Manual + Alarm",
    "Schedule",
    "Manual + Schedule",
    "Alarm + Schedule",
    "Manual + Alarm + Schedule",
    "COSEC",
    "COSEC + Manual",
    "COSEC + Alarm",
    "COSEC + Manual + Alarm",
    "COSEC + Schedule",
    "COSEC + Schedule + Manual",
    "COSEC + Schedule + Alarm",
    "All"
};

static const QStringList manualBackupLocations = QStringList ()
        << "USB Device"
        << "Network Drive 1"
        << "Network Drive 2";


static quint8 secRecordIndex[MAX_REC_TO_BACKUP] = { INVALID_FEILD, INVALID_FEILD, INVALID_FEILD, INVALID_FEILD,
                                                    INVALID_FEILD, INVALID_FEILD, INVALID_FEILD, INVALID_FEILD };

PlaybackSearch::PlaybackSearch(QWidget * parent)
    :BackGround((ApplController::getXPosOfScreen() + ((ApplController::getWidthOfScreen() - PB_SEARCH_MAIN_RECT_WIDTH) / 2)),
                (ApplController::getYPosOfScreen() + ((ApplController::getHeightOfScreen() - PB_SEARCH_MAIN_RECT_HEIGHT) / 2)),
                PB_SEARCH_MAIN_RECT_WIDTH,
                (PB_SEARCH_MAIN_RECT_HEIGHT - PB_SEARCH_HEADER_RECT_HEIGHT),
                BACKGROUND_TYPE_1,
                ASYN_PLAYBACK_BUTTON,
                parent,
                PB_SEARCH_HEADER_RECT_WIDTH,
                PB_SEARCH_HEADER_RECT_HEIGHT,
                PB_SEARCH_HEADING)
{
    applController = ApplController::getInstance ();
    payloadLib = new PayloadLib();

    backUpSingleRecIndex = 0;
    currPage = 0;
    totalCam = 0;
    totalPage = 0;
    backupSingleRecord = NULL;
    backupRecords = NULL;
    isBackupReqSent = false;
    totalRec = 0;
    isQuickBackupon = false;
    m_currDevName = "";

    for(quint8 index = 0; index < MAX_CAMERAS; index++)
    {
        m_cameraIndex[index] = 0;
    }

    channelList.append ("All");
    createDefaultComponent ();

    processBar = new ProcessBar(0, PB_SEARCH_HEADER_RECT_HEIGHT,
                                PB_SEARCH_MAIN_RECT_WIDTH,
                                PB_SEARCH_MAIN_RECT_HEIGHT -PB_SEARCH_HEADER_RECT_HEIGHT,
                                SCALE_WIDTH(15), this);

    processBar->loadProcessBar();

    infoPage = new InfoPage(0, 0,
                            PB_SEARCH_MAIN_RECT_WIDTH,
                            PB_SEARCH_MAIN_RECT_HEIGHT,
                            INFO_PLABACK_SEARCH,
                            this);
    connect (infoPage,
             SIGNAL(sigInfoPageCnfgBtnClick(int)),
             this,
             SLOT(slotInfoPageCnfgBtnClick(int)));

    for(quint8 index = 0; index < MAX_REC_ALLOWED; index++)
    {
        recSelect[index] = false;
    }

    selAllOptionOfWhichPage = INVALID_FEILD;
    totalTick = 0;
    m_currElement = PB_SEARCH_DEVICE_SPINBOX;
    m_elementList[m_currElement]->forceActiveFocus ();
    slotSpinboxValueChanged(devNameDropDownBox->getCurrValue(), PB_SEARCH_DEVICE_SPINBOX);
}

PlaybackSearch :: ~PlaybackSearch()
{
    deleteBackupPage ();

    disconnect(m_mainCloseButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete processBar;
    delete payloadLib;

    disconnect (infoPage,
                SIGNAL(sigInfoPageCnfgBtnClick(int)),
                this,
                SLOT(slotInfoPageCnfgBtnClick(int)));
    delete infoPage;

    disconnect (devNameDropDownBox,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotSpinboxValueChanged(QString,quint32)));
    disconnect(devNameDropDownBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete devNameDropDownBox;

    disconnect(m_recDriveListDropdown,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    disconnect (m_recDriveListDropdown,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotSpinboxValueChanged(QString,quint32)));
    delete m_recDriveListDropdown;

    disconnect(startDateCalender,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));

	 disconnect(startDateCalender,
            SIGNAL(sigDateChanged()),
            this,
            SLOT(slotDateChanged()));

    delete startDateCalender;

    disconnect(startTimeSpinbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete startTimeSpinbox;

    disconnect(endDateCalender,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete endDateCalender;

    disconnect(endTimeSpinbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete endTimeSpinbox;

    disconnect(recTypeDropDownBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete recTypeDropDownBox;

    disconnect(channelDropDownBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete channelDropDownBox;

    disconnect (searchBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotControlBtnclick(int)));
    disconnect(searchBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete searchBtn;

    delete tableCellHeaderBg;

    for(quint8 index = 0; index < MAX_PB_SEARCH_TABLE_COL; index++)
    {
        delete headingTableCell[index];
    }

    disconnect (selAllCheckbox,
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotAllOptionSelBtnClick(OPTION_STATE_TYPE_e,int)));
    disconnect(selAllCheckbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete selAllCheckbox;

    for(quint8 index = 0; index < MAX_PB_SEARCH_TABLE_COL; index++)
    {
        delete headingTextLabel[index];
    }

    delete tableCellDataBg;

    for(quint8 row = 0; row < MAX_PB_SEARCH_TABLE_ROW; row++)
    {
        disconnect (recSelCheckboxes[row],
                    SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                    this,
                    SLOT(slotAllOptionSelBtnClick(OPTION_STATE_TYPE_e,int)));
        disconnect(recSelCheckboxes[row],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpadateCurrentElement(int)));
        delete recSelCheckboxes[row];

        disconnect (playBtns[row],
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotControlBtnclick(int)));
        disconnect(playBtns[row],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpadateCurrentElement(int)));
        delete playBtns[row];

        for(quint8 col = 0; col < MAX_PB_SEARCH_TABLE_COL; col++)
        {
            delete recDataTableCell[row][col];
            if(col < (MAX_PB_SEARCH_TABLE_COL - 1))
                delete recDataTextLabel[row][col];
        }
    }

    delete pageNoReadOnly;

    disconnect (firstPageBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotControlBtnclick(int)));
    disconnect(firstPageBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete firstPageBtn;

    disconnect (prevPageBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotControlBtnclick(int)));
    disconnect(prevPageBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete prevPageBtn;

    disconnect (nextPageBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotControlBtnclick(int)));
    disconnect(nextPageBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete nextPageBtn;

    disconnect (lastPageBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotControlBtnclick(int)));
    disconnect(lastPageBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete lastPageBtn;

    disconnect(backupBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete backupBtn;
}

void PlaybackSearch::createDefaultComponent ()
{
    m_elementList[PB_SEARCH_CLOSE_BTN] = m_mainCloseButton;
    m_mainCloseButton->setIndexInPage(PB_SEARCH_CLOSE_BTN);
    connect(m_mainCloseButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));

    QMap<quint8, QString> deviceMapList;
    applController->GetDevNameDropdownMapList(deviceMapList);

    devNameDropDownBox = new DropDown(SCALE_WIDTH(21),
                                      SCALE_HEIGHT(70),
                                      BGTILE_LARGE_SIZE_WIDTH + SCALE_WIDTH(126),
                                      BGTILE_HEIGHT,
                                      PB_SEARCH_DEVICE_SPINBOX,
                                      DROPDOWNBOX_SIZE_200,
                                      labelStr[PB_SEARCH_DEVICE_SPINBOX],
                                      deviceMapList, this, "", false,
                                      SCALE_WIDTH(38), UP_LAYER);
    m_elementList[PB_SEARCH_DEVICE_SPINBOX] = devNameDropDownBox;
    connect(devNameDropDownBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect (devNameDropDownBox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotSpinboxValueChanged(QString,quint32)));

    QMap<quint8, QString>  driveMapList;

    for(quint8 index = 0; index <  recDriveList.length (); index++)
    {
        driveMapList.insert (index,recDriveList.at (index));
    }

    m_recDriveListDropdown = new DropDown(devNameDropDownBox->x() + SCALE_WIDTH(575),
                                          devNameDropDownBox->y () ,
                                          BGTILE_LARGE_SIZE_WIDTH + SCALE_WIDTH(126) ,
                                          BGTILE_HEIGHT,
                                          PB_SEARCH_REC_DRIVE_DROPBOX,
                                          DROPDOWNBOX_SIZE_200,
                                          labelStr[PB_SEARCH_REC_DRIVE_DROPBOX],
                                          driveMapList,this,"",false,0,NO_LAYER);

    m_elementList[PB_SEARCH_REC_DRIVE_DROPBOX] = m_recDriveListDropdown;
    connect(m_recDriveListDropdown,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect (m_recDriveListDropdown,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotSpinboxValueChanged(QString,quint32)));

    startDateCalender = new CalendarTile(devNameDropDownBox->x (),
                                         devNameDropDownBox->y () + devNameDropDownBox->height (),
                                         BGTILE_LARGE_SIZE_WIDTH + SCALE_WIDTH(126),
                                         BGTILE_HEIGHT,
                                         "", labelStr[PB_SEARCH_START_DATE],
                                         this, PB_SEARCH_START_DATE, false,
                                         SCALE_WIDTH(20), MIDDLE_TABLE_LAYER);

    m_elementList[PB_SEARCH_START_DATE] = startDateCalender;

    connect(startDateCalender,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));

	connect(startDateCalender,
            SIGNAL(sigDateChanged()),
            this,
            SLOT(slotDateChanged()));


    startTimeSpinbox = new ClockSpinbox(startDateCalender->x () + SCALE_WIDTH(310),
                                        startDateCalender->y () ,
                                        BGTILE_LARGE_SIZE_WIDTH + SCALE_WIDTH(126),
                                        BGTILE_HEIGHT,
                                        PB_SEARCH_START_TIME, CLK_SPINBOX_With_NO_SEC,
                                        labelStr[PB_SEARCH_START_TIME], 10,
                                        this, "", false,
                                        0, NO_LAYER);

    m_elementList[PB_SEARCH_START_TIME] = startTimeSpinbox;

    connect(startTimeSpinbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));

    endDateCalender = new CalendarTile(startDateCalender->x () + SCALE_WIDTH(565),
                                       startDateCalender->y () ,
                                       BGTILE_LARGE_SIZE_WIDTH + SCALE_WIDTH(126),
                                       BGTILE_HEIGHT,
                                       "", labelStr[PB_SEARCH_END_DATE],
                                       this, PB_SEARCH_END_DATE,
                                       false, SCALE_WIDTH(20), NO_LAYER);

    m_elementList[PB_SEARCH_END_DATE] = endDateCalender;

    connect(endDateCalender,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));

    endTimeSpinbox = new ClockSpinbox(startDateCalender->x () + SCALE_WIDTH(850),
                                      startDateCalender->y () ,
                                      BGTILE_LARGE_SIZE_WIDTH + SCALE_WIDTH(126),
                                      BGTILE_HEIGHT, PB_SEARCH_END_TIME,
                                      CLK_SPINBOX_With_NO_SEC,
                                      labelStr[PB_SEARCH_END_TIME], 10,
                                      this, "", false, 0, NO_LAYER);

    m_elementList[PB_SEARCH_END_TIME] = endTimeSpinbox;

    connect(endTimeSpinbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));

    recTypeDropDownBox = new DropDown(startDateCalender->x (),
                                      startDateCalender->y ()+ startDateCalender->height (),
                                      BGTILE_LARGE_SIZE_WIDTH + SCALE_WIDTH(126),
                                      BGTILE_HEIGHT,
                                      PB_SEARCH_REC_TYPE_SPINBOX,
                                      DROPDOWNBOX_SIZE_200,
                                      labelStr[PB_SEARCH_REC_TYPE_SPINBOX],
                                      recTypeStrMapList, this, "", false,
                                      SCALE_WIDTH(65), MIDDLE_TABLE_LAYER);
    m_elementList[PB_SEARCH_REC_TYPE_SPINBOX] = recTypeDropDownBox;
    connect(recTypeDropDownBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));

    QMap<quint8, QString>  channelMapList;

    for(quint8 index = 0; index < channelList.length (); index++)
    {
        channelMapList.insert (index,channelList.at (index));
    }

    channelDropDownBox = new DropDown(recTypeDropDownBox->x () + SCALE_WIDTH(575),
                                      recTypeDropDownBox->y () ,
                                      BGTILE_LARGE_SIZE_WIDTH + SCALE_WIDTH(126),
                                      BGTILE_HEIGHT,
                                      PB_SEARCH_CHANNEL_SPINBOX,
                                      DROPDOWNBOX_SIZE_200,
                                      labelStr[PB_SEARCH_CHANNEL_SPINBOX],
                                      channelMapList, this, "", false,
                                      0, NO_LAYER);
    m_elementList[PB_SEARCH_CHANNEL_SPINBOX] = channelDropDownBox;
    connect(channelDropDownBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));

    searchBtn = new ControlButton(SEARCH_BUTTON_INDEX,
                                  recTypeDropDownBox->x (),
                                  recTypeDropDownBox->y () + recTypeDropDownBox->height (),
                                  BGTILE_LARGE_SIZE_WIDTH + SCALE_WIDTH(126),
                                  BGTILE_HEIGHT,
                                  this, DOWN_LAYER, SCALE_WIDTH(975),
                                  labelStr[PB_SEARCH_SEARCH_BTN],
                                  true, PB_SEARCH_SEARCH_BTN);
    m_elementList[PB_SEARCH_SEARCH_BTN] = searchBtn;
    connect(searchBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect (searchBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotControlBtnclick(int)));

    tableCellHeaderBg = new BgTile(searchBtn->x (),
                                   searchBtn->y () + searchBtn->height () + SCALE_HEIGHT(5),
                                   BGTILE_LARGE_SIZE_WIDTH + SCALE_WIDTH(126),
                                   BGTILE_HEIGHT,
                                   UP_LAYER, this);

    headingTableCell[0] = new TableCell(tableCellHeaderBg->x () + SCALE_WIDTH(10),
                                        tableCellHeaderBg->y () + SCALE_HEIGHT(10),
                                        ((TABELCELL_EXTRASMALL_SIZE_WIDTH - SCALE_WIDTH(6)) - SCALE_WIDTH(2)),
                                        TABELCELL_HEIGHT, this, true);
    headingTableCell[1] = new TableCell(headingTableCell[0]->x () + headingTableCell[0]->width (),
                                        headingTableCell[0]->y (),
                                        TABELCELL_MEDIUM_SIZE_WIDTH - SCALE_WIDTH(4),
                                        TABELCELL_HEIGHT, this, true);
    headingTableCell[2] = new TableCell(headingTableCell[1]->x () + headingTableCell[1]->width (),
                                        headingTableCell[1]->y (),
                                        TABELCELL_MEDIUM_SIZE_WIDTH - SCALE_WIDTH(4),
                                        TABELCELL_HEIGHT, this, true);
    headingTableCell[3] = new TableCell(headingTableCell[2]->x () + headingTableCell[2]->width (),
                                        headingTableCell[2]->y (),
                                        TABELCELL_EXTRASMALL_SIZE_WIDTH - SCALE_WIDTH(4),
                                        TABELCELL_HEIGHT, this, true);
    headingTableCell[4] = new TableCell(headingTableCell[3]->x () + headingTableCell[3]->width (),
                                        headingTableCell[3]->y (),
                                        SCALE_WIDTH(290),
                                        TABELCELL_HEIGHT, this, true);
    headingTableCell[5] = new TableCell(headingTableCell[4]->x () + headingTableCell[4]->width (),
                                        headingTableCell[4]->y (),
                                        SCALE_WIDTH(152),
                                        TABELCELL_HEIGHT, this, true);

    headingTableCell[6] = new TableCell(headingTableCell[5]->x () + headingTableCell[5]->width (),
                                        headingTableCell[5]->y (),
                                        SCALE_WIDTH(56),
                                        TABELCELL_HEIGHT, this, true);

    headingTextLabel[0] = new TextLabel(headingTableCell[0]->x () + SCALE_WIDTH(50),
                                        headingTableCell[0]->y () + (headingTableCell[0]->height ())/2,
                                        SCALE_FONT(SUB_HEADING_FONT_SIZE),
                                        tableCellHeadingStr[0], this,
                                        HIGHLITED_FONT_COLOR,
                                        NORMAL_FONT_FAMILY,
                                        ALIGN_START_X_CENTRE_Y, 0, 0, (headingTableCell[0]->width() - SCALE_WIDTH(8)));

    selAllCheckbox = new OptionSelectButton(headingTableCell[0]->x () + SCALE_WIDTH(15),
                                            headingTableCell[0]->y (),
                                            BGTILE_MEDIUM_SIZE_WIDTH,
                                            SCALE_HEIGHT(30),
                                            CHECK_BUTTON_INDEX,
                                            this, NO_LAYER, "",
                                            "",0, PB_SEARCH_SEL_ALL, false);
    m_elementList[PB_SEARCH_SEL_ALL] = selAllCheckbox;
    connect(selAllCheckbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect (selAllCheckbox,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotAllOptionSelBtnClick(OPTION_STATE_TYPE_e,int)));

    for(quint8 index = 1; index < MAX_PB_SEARCH_TABLE_COL; index++)
    {
        headingTextLabel[index] = new TextLabel(headingTableCell[index]->x () + SCALE_WIDTH(10),
                                                headingTableCell[index]->y () +
                                                (headingTableCell[index]->height ())/2,
                                                SCALE_FONT(SUB_HEADING_FONT_SIZE),
                                                tableCellHeadingStr[index],
                                                this,
                                                HIGHLITED_FONT_COLOR,
                                                NORMAL_FONT_FAMILY,
                                                ALIGN_START_X_CENTRE_Y, 0, 0, (headingTableCell[index]->width() - SCALE_WIDTH(8)));
    }

    tableCellDataBg = new BgTile(tableCellHeaderBg->x (),
                                 tableCellHeaderBg->y () + tableCellHeaderBg->height (),
                                 BGTILE_LARGE_SIZE_WIDTH  + SCALE_WIDTH(126),
                                 SCALE_HEIGHT(300) , MIDDLE_LAYER, this);

    for(quint8 row = 0; row < MAX_PB_SEARCH_TABLE_ROW; row++)
    {
        for(quint8 col = 0; col < MAX_PB_SEARCH_TABLE_COL; col++)
        {
            recDataTableCell[row][col] = new TableCell(headingTableCell[col]->x (),
                                                       headingTableCell[col]->y () + headingTableCell[col]->height () +
                                                       (row * TABELCELL_HEIGHT),
                                                       headingTableCell[col]->width () - 1,
                                                       headingTableCell[col]->height (), this);

            if(col < (MAX_PB_SEARCH_TABLE_COL - 1))
            {
                recDataTextLabel[row][col] = new TextLabel(((col == 0)?(recDataTableCell[row][col]->x () + SCALE_WIDTH(25))
                                                                     :(recDataTableCell[row][col]->x () + SCALE_WIDTH(10))),
                                                           recDataTableCell[row][col]->y () + (recDataTableCell[row][col]->height ()/2),
                                                           NORMAL_FONT_SIZE,
                                                           "", this, NORMAL_FONT_COLOR,
                                                           NORMAL_FONT_FAMILY,
                                                           ALIGN_START_X_CENTRE_Y);
            }
        }

        recSelCheckboxes[row] = new OptionSelectButton(recDataTableCell[row][0]->x () + SCALE_WIDTH(15),
                                                       recDataTableCell[row][0]->y (),
                                                       BGTILE_MEDIUM_SIZE_WIDTH,
                                                       SCALE_HEIGHT(30),
                                                       CHECK_BUTTON_INDEX,
                                                       this, NO_LAYER, "","",
                                                       0, (PB_SEARCH_SEL_1ST + (row *2)), false);
        m_elementList[PB_SEARCH_SEL_1ST + (row *2)] = recSelCheckboxes[row];
        connect(recSelCheckboxes[row],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpadateCurrentElement(int)));
        connect (recSelCheckboxes[row],
                 SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                 this,
                 SLOT(slotAllOptionSelBtnClick(OPTION_STATE_TYPE_e,int)));
        recSelCheckboxes[row]->setVisible (false);

        playBtns[row] = new ControlButton(PLAY_BUTTON_INDEX,
                                          recDataTableCell[row][6]->x () + SCALE_WIDTH(15),
                                          recDataTableCell[row][6]->y (),
                                          BGTILE_MEDIUM_SIZE_WIDTH,
                                          SCALE_HEIGHT(30), this, NO_LAYER, 0,"",
                                          false, (PB_SEARCH_PLAY_1ST + (row *2)));
        m_elementList[PB_SEARCH_PLAY_1ST + (row *2)] = playBtns[row];
        connect(playBtns[row],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpadateCurrentElement(int)));
        connect (playBtns[row],
                 SIGNAL(sigButtonClick(int)),
                 this,
                 SLOT(slotControlBtnclick(int)));
        playBtns[row]->setVisible (false);
    }

    pageNoReadOnly = new ReadOnlyElement(tableCellDataBg->x (),
                                         tableCellDataBg->y ()+tableCellDataBg->height (),
                                         BGTILE_LARGE_SIZE_WIDTH + SCALE_WIDTH(126),
                                         (BGTILE_HEIGHT + SCALE_HEIGHT(10)),
                                         SCALE_WIDTH(READONLY_SMALL_WIDTH),
                                         READONLY_HEIGHT,
                                         "", this, DOWN_LAYER,
                                         SCALE_WIDTH(500), -1);

    firstPageBtn = new ControlButton(FIRSTPAGE_BUTTON_INDEX,
                                     SCALE_WIDTH(435),
                                     pageNoReadOnly->y () + SCALE_HEIGHT(5),
                                     BGTILE_LARGE_SIZE_WIDTH + SCALE_WIDTH(126),
                                     BGTILE_HEIGHT,
                                     this, NO_LAYER, 0,
                                     "", false, PB_SEARCH_FIRST_PAGE);
    m_elementList[PB_SEARCH_FIRST_PAGE] = firstPageBtn;
    connect(firstPageBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect (firstPageBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotControlBtnclick(int)));


    prevPageBtn = new ControlButton(PREVIOUS_BUTTON_INDEX,
                                    firstPageBtn->x () + firstPageBtn->width () + SCALE_WIDTH(10),
                                    pageNoReadOnly->y () + SCALE_HEIGHT(5),
                                    BGTILE_LARGE_SIZE_WIDTH + SCALE_WIDTH(126),
                                    BGTILE_HEIGHT,
                                    this, NO_LAYER, 0,
                                    "", false, PB_SEARCH_PREV_PAGE);
    m_elementList[PB_SEARCH_PREV_PAGE] = prevPageBtn;
    connect(prevPageBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect (prevPageBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotControlBtnclick(int)));

    nextPageBtn = new ControlButton(NEXT_BUTTON_INDEX,
                                    SCALE_WIDTH(625),
                                    pageNoReadOnly->y () + SCALE_HEIGHT(5),
                                    BGTILE_LARGE_SIZE_WIDTH + SCALE_WIDTH(126),
                                    BGTILE_HEIGHT,
                                    this, NO_LAYER, 0,
                                    "", false, PB_SEARCH_NEXT_PAGE);
    m_elementList[PB_SEARCH_NEXT_PAGE] = nextPageBtn;
    connect(nextPageBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect (nextPageBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotControlBtnclick(int)));

    lastPageBtn = new ControlButton(LAST_BUTTON_INDEX,
                                    nextPageBtn->x ()+ nextPageBtn->width () + SCALE_WIDTH(10),
                                    pageNoReadOnly->y () + SCALE_HEIGHT(5),
                                    BGTILE_LARGE_SIZE_WIDTH + SCALE_WIDTH(126),
                                    BGTILE_HEIGHT,
                                    this, NO_LAYER, 0,
                                    "", false, PB_SEARCH_LAST_PAGE);
    m_elementList[PB_SEARCH_LAST_PAGE] = lastPageBtn;
    connect(lastPageBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect (lastPageBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotControlBtnclick(int)));

    backupBtn = new CnfgButton(CNFGBUTTON_MEDIAM,
                               SCALE_WIDTH(1040),
                               pageNoReadOnly->y () + (pageNoReadOnly->height ()/2),
                               "Backup",
                               this, PB_SEARCH_BACKUP_BTN,false);
    m_elementList[PB_SEARCH_BACKUP_BTN] = backupBtn;
    connect(backupBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect (backupBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotBackupBtnClicked(int)));
}

void PlaybackSearch::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void PlaybackSearch::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}


void PlaybackSearch::navigationKeyPressed (QKeyEvent *event)
{
        event->accept();
}

void PlaybackSearch::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    m_currElement = PB_SEARCH_CLOSE_BTN;
    m_elementList[PB_SEARCH_CLOSE_BTN]->forceActiveFocus();
}

void PlaybackSearch::showEvent (QShowEvent *event)
{
    QWidget::showEvent (event);
    m_elementList[m_currElement]->forceActiveFocus ();
}

void PlaybackSearch::takeLeftKeyAction()
{
    do
    {
        m_currElement = (m_currElement - 1 + MAX_PB_SEARCH_ELEMENTS)
                % MAX_PB_SEARCH_ELEMENTS;
    }while(!m_elementList[m_currElement]->getIsEnabled());

    m_elementList[m_currElement]->forceActiveFocus();
}

void PlaybackSearch::takeRightKeyAction()
{
    do
    {
        m_currElement = (m_currElement + 1)
                % MAX_PB_SEARCH_ELEMENTS;
    }while(!m_elementList[m_currElement]->getIsEnabled());

    m_elementList[m_currElement]->forceActiveFocus();
}

void PlaybackSearch::getDevDateTime()
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = GET_DATE_TIME;

    applController->processActivity(m_currDevName, DEVICE_COMM, param);
}

void PlaybackSearch::sendBackupCmd(QString startTime,
                                   QString endTime)
{
    for(quint8 index = 0; index < MAX_REC_ALLOWED; index++)
    {
        recSelect[index] = false;
    }

    totalTick = 0;
    selAllOptionOfWhichPage = INVALID_FEILD;
    selAllCheckbox->changeState (OFF_STATE);

    showTableData ();

    payloadLib->setCnfgArrayAtIndex (0, startTime);
    payloadLib->setCnfgArrayAtIndex (1, endTime);
    payloadLib->setCnfgArrayAtIndex (2, playbackRecData[backUpSingleRecIndex].camNo);
    payloadLib->setCnfgArrayAtIndex (3, playbackRecData[backUpSingleRecIndex].overlap);
    payloadLib->setCnfgArrayAtIndex (4, playbackRecData[backUpSingleRecIndex].hddIndicator);
    payloadLib->setCnfgArrayAtIndex (5, 1);
    payloadLib->setCnfgArrayAtIndex (6, playbackRecData[backUpSingleRecIndex].partionIndicator);
    payloadLib->setCnfgArrayAtIndex (7, playbackRecData[backUpSingleRecIndex].recDriveIndex);
    payloadLib->setCnfgArrayAtIndex (8, 3);

    QString payloadString = payloadLib->createDevCmdPayload(9);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = BKUP_RCD;
    param->payload = payloadString;

    if(applController->processActivity(m_currDevName, DEVICE_COMM, param))
    {
        isBackupReqSent = true;
    }
}

void PlaybackSearch::sendMultipleRecBackupCmd()
{
    quint8 index = 0;

    for(; index < totalTick; index++)
    {
        if((index < MAX_REC_TO_BACKUP) && (secRecordIndex[index] < MAX_REC_ALLOWED))
        {
            payloadLib->setCnfgArrayAtIndex (0,playbackRecData[secRecordIndex[index]].startTime);
            payloadLib->setCnfgArrayAtIndex (1,playbackRecData[secRecordIndex[index]].endTime);
            payloadLib->setCnfgArrayAtIndex (2,playbackRecData[secRecordIndex[index]].camNo);
            payloadLib->setCnfgArrayAtIndex (3,playbackRecData[secRecordIndex[index]].overlap);
            payloadLib->setCnfgArrayAtIndex (4,playbackRecData[secRecordIndex[index]].hddIndicator);
            payloadLib->setCnfgArrayAtIndex (5,1);
            payloadLib->setCnfgArrayAtIndex (6,playbackRecData[secRecordIndex[index]].partionIndicator);
            payloadLib->setCnfgArrayAtIndex (7,playbackRecData[secRecordIndex[index]].recDriveIndex);
             payloadLib->setCnfgArrayAtIndex (8, 3);

            secRecordIndex[index] = INVALID_FEILD;
            break;
        }
    }

    if(index == totalTick)
    {
        for(quint8 index = 0; index < MAX_REC_ALLOWED; index++)
        {
            recSelect[index] = false;
        }

        totalTick = 0;
        selAllOptionOfWhichPage = INVALID_FEILD;
        selAllCheckbox->changeState (OFF_STATE);

        showTableData ();

        deleteBackupPage();
    }
    else
    {
        QString payloadString = payloadLib->createDevCmdPayload(9);

        DevCommParam* param = new DevCommParam();
        param->msgType = MSG_SET_CMD;
        param->cmdType = BKUP_RCD;
        param->payload = payloadString;

        if(applController->processActivity(m_currDevName, DEVICE_COMM, param))
        {
            isBackupReqSent = true;
        }
    }
}

void PlaybackSearch::fillMultipleRecBackupCmd ()
{
    quint8 selRec = 0, index = 0;

    for(selRec = 0; selRec < MAX_REC_ALLOWED; selRec++)
    {
        if(recSelect[selRec] == true)
        {
            secRecordIndex[index] = selRec;
            index++;
            if(index == totalTick)
            {
                break;
            }
        }
    }

    for(;index < MAX_REC_TO_BACKUP; index++)
    {
        secRecordIndex[index] = INVALID_FEILD;
    }
}

void PlaybackSearch::sendStopBackupCmd()
{
    payloadLib->setCnfgArrayAtIndex (0, 0); // 0 for Manual Backup

    QString payloadString = payloadLib->createDevCmdPayload(1);

    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = STP_BCKUP;
    param->payload = payloadString;

    if(applController->processActivity(m_currDevName, DEVICE_COMM, param))
    {
        isBackupReqSent = false;
    }
}


void PlaybackSearch::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    DDMMYY_PARAM_t dateTimeParam;
    if(deviceName == m_currDevName)
    {
        switch(param->msgType)
        {
        case MSG_SET_CMD:
        {
            switch(param->cmdType)
            {
            case SEARCH_RCD_DATA:
            case SEARCH_RCD_ALL_DATA:
                processBar->unloadProcessBar ();
                if((param->deviceStatus == CMD_SUCCESS) ||
                        (param->deviceStatus == CMD_MORE_DATA))
                {
                    payloadLib->parseDevCmdReply (false, param->payload);

                    quint8 tempCamNumber,tempRecFound = 0;
                    quint32 totalReplyFields = 0;
                    PlaybackRecordData tempRecordData;
                    totalReplyFields = payloadLib->getTotalReplyFields();
                    totalRec = (payloadLib->getTotalCmdFields () / totalReplyFields);

                    m_cameraList.clear();

                    applController->GetAllCameraNamesOfDevice(m_currDevName,
                                                              m_cameraList,
                                                              PLAYBACK_CAM_LIST_TYPE,
                                                              &m_cameraIndex[0]);

                    for(quint8 index = 0; index < MAX_REC_ALLOWED; index++)
                    {
                        playbackRecData[index].camNo = MAX_CAMERAS;
                    }

                    for(quint8 index = 0; index < totalRec; index++)
                    {
                        tempCamNumber = payloadLib->getCnfgArrayAtIndex (FIELD_CAM_NO + (index *totalReplyFields)).toUInt ();

                        if(isCameraRightsAvailable(tempCamNumber))
                        {
                            playbackRecData[tempRecFound].startTime = payloadLib->getCnfgArrayAtIndex (FIELD_STRAT_TIME + (index *totalReplyFields)).toString ();
                            playbackRecData[tempRecFound].endTime = payloadLib->getCnfgArrayAtIndex (FIELD_END_TIME + (index *totalReplyFields)).toString ();
                            playbackRecData[tempRecFound].camNo = tempCamNumber;
                            playbackRecData[tempRecFound].evtType = payloadLib->getCnfgArrayAtIndex (FIELD_EVT_TYPE + (index *totalReplyFields)).toUInt ();
                            playbackRecData[tempRecFound].overlap = payloadLib->getCnfgArrayAtIndex (FIELD_OVERLAP + (index *totalReplyFields)).toUInt ();
                            playbackRecData[tempRecFound].hddIndicator = payloadLib->getCnfgArrayAtIndex (FIELD_HDD + (index *totalReplyFields)).toUInt ();
                            playbackRecData[tempRecFound].partionIndicator = payloadLib->getCnfgArrayAtIndex (FIELD_PARTION + (index *totalReplyFields)).toUInt ();
                            if( totalReplyFields > 8)
                            {
                                playbackRecData[tempRecFound].recDriveIndex = payloadLib->getCnfgArrayAtIndex (FIELD_STORAGE + (index *totalReplyFields)).toUInt ();
                            }
                            else
                            {
                                playbackRecData[tempRecFound].recDriveIndex = m_recDriveListDropdown->getIndexofCurrElement() - 1;
                            }
                            playbackRecData[tempRecFound].deviceName = m_currDevName;
                            tempRecFound++;
                        }
                    }

                    totalRec = tempRecFound;  // if rights not given to all recording then get rights record data
                    totalPage = ((totalRec % MAX_REC_IN_ONE_PAGE) == 0)?(totalRec/MAX_REC_IN_ONE_PAGE):(totalRec/MAX_REC_IN_ONE_PAGE + 1);
                    tempData.tempType = recTypeDropDownBox->getIndexofCurrElement ();
                    tempData.tempSource =  m_recDriveListDropdown->getIndexofCurrElement ();

                    for(quint8 index = totalRec; index < MAX_REC_ALLOWED; index++)
                    {
                        playbackRecData[index] = tempRecordData;
                    }

                    for(quint8 index = 0; index < MAX_REC_ALLOWED; index++)
                    {
                        recSelect[index] = false;
                    }

                    currPage = 0;
                    totalTick = 0;

                    selAllOptionOfWhichPage = INVALID_FEILD;
                    selAllCheckbox->changeState (OFF_STATE);
                    showTableData ();

                    if(param->deviceStatus == CMD_MORE_DATA)
                    {
                        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(PB_SRCH_MORE_REC_MSG));
                    }
                }
                else
                {
                    infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                }
                emit sigPreviousRecords(playbackRecData,totalRec,&tempData);
                break;

            case GET_DATE_TIME:
                processBar->unloadProcessBar ();
                if(param->deviceStatus == CMD_SUCCESS)
                {
                    if(totalRec == 0)
                    {
                        payloadLib->parseDevCmdReply(true, param->payload);

                        QDateTime dt = QDateTime::fromString (payloadLib->getCnfgArrayAtIndex(0).toString(),
                                                              "ddMMyyyyHHmmss");
                        QDate date = dt.date ();
                        QTime time = dt.time ();

                        dateTimeParam.date  = date.day ();
                        dateTimeParam.month = date.month ();
                        dateTimeParam.year  = date.year ();
                        endDateCalender->setDDMMYY (&dateTimeParam);
                        endDataCalender = dateTimeParam;
                        endTimeSpinbox->assignValue (time.hour (), time.minute ());

                        // show start time 1 Hr Before
                        dt = dt.addSecs (-3600);
                        date = dt.date ();
                        time = dt.time ();
                        dateTimeParam.date  = date.day ();
                        dateTimeParam.month = date.month ();
                        dateTimeParam.year  = date.year ();
                        startDateCalender->setDDMMYY (&dateTimeParam);
                        PreviousDataCalender = dateTimeParam;
                        startTimeSpinbox->assignValue (time.hour (), time.minute ());
                    }
                }
                else
                {
                    infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                }
                break;

            case BKUP_RCD:
                if(param->deviceStatus != CMD_SUCCESS)
                {
                    deleteBackupPage ();

                    if(param->deviceStatus == CMD_NO_DISK_FOUND)
                    {
                        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(PB_SRCH_BKUP_DEV_ERR_MSG));
                    }
                    else
                    {
                        infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                    }
                }
                break;

            case STP_BCKUP:
                if(param->deviceStatus != CMD_SUCCESS)
                {
                    infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                }
                break;

            case GET_MAN_BKP_LOC:
                quint8 tempLocation;
                processBar->unloadProcessBar();
                if(param->deviceStatus != CMD_SUCCESS)
                {
                    tempLocation = 0;
                }
                else
                {
                    payloadLib->parseDevCmdReply(true, param->payload);
                    tempLocation = payloadLib->getCnfgArrayAtIndex(0).toUInt();
                    if(tempLocation >= manualBackupLocations.size ())
                    {
                        tempLocation = 0;
                    }
                }

                if(backupSingleRecord != NULL)
                {
                    backupSingleRecord->updateBackUpLocation(manualBackupLocations.at (tempLocation));
                }

                if(backupRecords != NULL)
                {
                    backupRecords->updateBackUpLocation(manualBackupLocations.at (tempLocation));
                }
                break;

            default:
                break;
            }
        }
            break;

        default:
            break;
        }
    }
    else
    {
        processBar->unloadProcessBar ();
    }
}

void PlaybackSearch::getManualBackupLocation()
{
    /* Create payload to send cmd */
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = GET_MAN_BKP_LOC;
    processBar->loadProcessBar();
    applController->processActivity(m_currDevName, DEVICE_COMM, param);
}

void PlaybackSearch::showTableData()
{
    quint8 recordOnPage = 0,tempCount = 0;
    QString startdateTimeStr, enddateTimeStr;

    selAllCheckbox->setIsEnabled (true);
    backupBtn->setIsEnabled ((totalRec >= 1));

    if(totalRec > 0)                          // if no record found then not count recordOnPage
    {
        if(totalRec < (MAX_REC_IN_ONE_PAGE*(currPage + 1)))
        {
            recordOnPage = totalRec - ((MAX_REC_IN_ONE_PAGE*(currPage)) );
        }
        else
        {
            recordOnPage = MAX_REC_IN_ONE_PAGE;
        }
    }

    pageNoReadOnly->changeValue (QString("Page") + QString(" %1").arg (currPage + 1));

    for(quint8 recIndex = 0; recIndex < recordOnPage; recIndex++)
    {
        quint8 feildIndex = recIndex + (MAX_REC_IN_ONE_PAGE*currPage);

        if(!recSelCheckboxes[recIndex]->isEnabled ())
            recSelCheckboxes[recIndex]->setIsEnabled (true);

        if(!recSelCheckboxes[recIndex]->isVisible ())
            recSelCheckboxes[recIndex]->setVisible (true);

        if(!playBtns[recIndex]->isEnabled ())
            playBtns[recIndex]->setIsEnabled (true);

        if(!playBtns[recIndex]->isVisible ())
            playBtns[recIndex]->setVisible (true);

        startdateTimeStr = playbackRecData[feildIndex].startTime;
        makeDatetimeInTableFormat (startdateTimeStr);
        enddateTimeStr = playbackRecData[feildIndex].endTime;
        makeDatetimeInTableFormat (enddateTimeStr);

        recDataTextLabel[recIndex][0]->changeText (QString("     %1").arg (feildIndex + 1));
        recDataTextLabel[recIndex][1]->changeText (startdateTimeStr);
        recDataTextLabel[recIndex][2]->changeText (enddateTimeStr);
        recDataTextLabel[recIndex][3]->changeText (QString("%1").arg (playbackRecData[feildIndex].camNo));
        recDataTextLabel[recIndex][4]->changeText (recTypeStr[playbackRecData[feildIndex].evtType]);
        recDataTextLabel[recIndex][5]->changeText(recDriveListforTable[playbackRecData[feildIndex].recDriveIndex]);


        recSelCheckboxes[recIndex]->changeState (recSelect[recIndex + (MAX_REC_IN_ONE_PAGE *currPage)] == true ? ON_STATE : OFF_STATE);

        if(recSelect[feildIndex] == true)
        {
            tempCount++;
        }
    }

    selAllCheckbox->changeState ((tempCount == recordOnPage) ?
                                     ON_STATE : OFF_STATE);
    if(tempCount == recordOnPage)
        selAllOptionOfWhichPage = currPage;

    if(recordOnPage != MAX_REC_IN_ONE_PAGE)
        clearTableData(recordOnPage);

    if(totalRec <= (MAX_REC_IN_ONE_PAGE * (currPage + 1)))
    {
        if(currPage != 0)
        {
            if(!prevPageBtn->hasFocus ())
            {
                prevPageBtn->setIsEnabled (true);
            }
            firstPageBtn->setIsEnabled (true);
        }

        if(nextPageBtn->hasFocus () || lastPageBtn->hasFocus ())
        {
            m_currElement = PB_SEARCH_PREV_PAGE;
            m_elementList[m_currElement]->forceActiveFocus ();
        }
        nextPageBtn->setIsEnabled (false);
        lastPageBtn->setIsEnabled (false);
    }
    else
    {
        if(currPage != totalPage)
        {
            if(!nextPageBtn->hasFocus ())
            {
                nextPageBtn->setIsEnabled (true);
            }
            lastPageBtn->setIsEnabled (true);
        }

        if(currPage > 0)
        {
            if(!prevPageBtn->hasFocus ())
            {
                prevPageBtn->setIsEnabled (true);
            }
            firstPageBtn->setIsEnabled (true);
        }
        else
        {
            if(prevPageBtn->hasFocus () || firstPageBtn->hasFocus ())
            {
                m_currElement = PB_SEARCH_NEXT_PAGE;
                m_elementList[m_currElement]->forceActiveFocus ();
            }
            prevPageBtn->setIsEnabled (false);
            firstPageBtn->setIsEnabled (false);
        }
    }
    update ();
}


void PlaybackSearch::showPrevTableData(PlaybackRecordData* prevRecordData, quint8 totalrec, TEMP_PLAYBACK_REC_DATA_t *prevRec )
{
    quint8 totalRecord1 = 0;

    DDMMYY_PARAM_t tempDateTimeParam;
    QDate date;
    QTime time;
    QString devName = LOCAL_DEVICE_NAME;

    for(quint16 index = 0; index < MAX_REC_ALLOWED; index++)
    {
        playbackRecData[index].clearPlaybackInfo();
    }
    if(totalrec > 0)
    {
        devName = prevRecordData[0].deviceName;
        for(totalRecord1 = 0; totalRecord1 < totalrec; totalRecord1++)
        {
            playbackRecData[totalRecord1].startTime = prevRecordData[totalRecord1].startTime;
            playbackRecData[totalRecord1].endTime =  prevRecordData[totalRecord1].endTime;
            playbackRecData[totalRecord1].camNo = prevRecordData[totalRecord1].camNo;
            playbackRecData[totalRecord1].evtType = prevRecordData[totalRecord1].evtType;
            playbackRecData[totalRecord1].overlap =  prevRecordData[totalRecord1].overlap;
            playbackRecData[totalRecord1].hddIndicator =  prevRecordData[totalRecord1].hddIndicator;
            playbackRecData[totalRecord1].partionIndicator = prevRecordData[totalRecord1].partionIndicator ;
            playbackRecData[totalRecord1].recDriveIndex = prevRecordData[totalRecord1].recDriveIndex;
            playbackRecData[totalRecord1].deviceName = prevRecordData[totalRecord1].deviceName;
        }
    }
    tempData.tempChannel = prevRec->tempChannel;
    tempData.tempType   =  prevRec->tempType;
    tempData.tempSource = prevRec->tempSource;
    tempData.tempStartTime = prevRec->tempStartTime;
    tempData.tempEndTime = prevRec->tempEndTime;
    tempData.currentPage = prevRec->currentPage;

    date = tempData.tempStartTime.date ();
    time = tempData.tempStartTime.time ();

    tempDateTimeParam.date = date.day ();
    tempDateTimeParam.month = date.month ();
    tempDateTimeParam.year = date.year ();

    startDateCalender->setDDMMYY (&tempDateTimeParam);
    PreviousDataCalender = tempDateTimeParam;
    startTimeSpinbox->assignValue (time.hour (), time.minute ());

    date = tempData.tempEndTime.date ();
    time = tempData.tempEndTime.time ();

    tempDateTimeParam.date = date.day ();
    tempDateTimeParam.month = date.month ();
    tempDateTimeParam.year = date.year ();

    endDateCalender->setDDMMYY (&tempDateTimeParam);
    endDataCalender = tempDateTimeParam;
    endTimeSpinbox->assignValue (time.hour (), time.minute ());


    totalRec = totalrec;
    totalPage = ((totalRec % MAX_REC_IN_ONE_PAGE) == 0)?(totalRec/MAX_REC_IN_ONE_PAGE):(totalRec/MAX_REC_IN_ONE_PAGE + 1);
    currPage =  tempData.currentPage;
    restoreSerchCriteria(devName);
    showTableData();
}

void PlaybackSearch::clearTableData (quint8 recIndex)
{
    for(; recIndex < MAX_REC_IN_ONE_PAGE; recIndex++)
    {
        if(recSelCheckboxes[recIndex]->isEnabled ())
            recSelCheckboxes[recIndex]->setIsEnabled (false);

        if(recSelCheckboxes[recIndex]->isVisible ())
            recSelCheckboxes[recIndex]->setVisible (false);

        if(playBtns[recIndex]->isEnabled ())
            playBtns[recIndex]->setIsEnabled (false);

        if(playBtns[recIndex]->isVisible ())
            playBtns[recIndex]->setVisible (false);

        recDataTextLabel[recIndex][0]->changeText ("");
        recDataTextLabel[recIndex][1]->changeText ("");
        recDataTextLabel[recIndex][2]->changeText ("");
        recDataTextLabel[recIndex][3]->changeText ("");
        recDataTextLabel[recIndex][4]->changeText ("");
        recDataTextLabel[recIndex][5]->changeText ("");
    }

    if(nextPageBtn->isEnabled ())
        nextPageBtn->setIsEnabled (false);

    if(lastPageBtn->isEnabled ())
        lastPageBtn->setIsEnabled (false);

    if(prevPageBtn->isEnabled ())
        prevPageBtn->setIsEnabled (false);

    if(firstPageBtn->isEnabled ())
        firstPageBtn->setIsEnabled (false);

    backupBtn->setIsEnabled ((totalRec >= 1));
}


void PlaybackSearch::makeDatetimeInTableFormat(QString &dateTimeStr)
{
    dateTimeStr.insert (2, "/");
    dateTimeStr.insert (5, "/");
    dateTimeStr.insert (10, " ");
    dateTimeStr.insert (13, ":");
    dateTimeStr.insert (16, ":");
}

void PlaybackSearch::setQuickBackupFlag(bool flag)
{
    isQuickBackupon = flag;
}
void PlaybackSearch::manualBkpSysEvtAction(QString devName, LOG_EVENT_STATE_e evtState)
{
    processBar->unloadProcessBar ();
    if(devName == m_currDevName)
    {
        // if event is Manual back up start
        if(evtState == EVENT_START)
        {
            if(backupRecords != NULL)
            {
                backupRecords->setPercentage (0);
            }

            if(backupSingleRecord != NULL)
            {
                backupSingleRecord->setPercentage (0);
            }
        }
        else if(evtState == EVENT_COMPLETE)
        {
            // delete fileCopystatus page
            if(backupRecords != NULL)
            {
                sendMultipleRecBackupCmd ();
                if(backupRecords != NULL)
                {
                    backupRecords->setPercentage (0);
                }
            }
            else
            {
                deleteBackupPage();
                infoPage->loadInfoPage (ValidationMessage::getValidationMessage(PB_SRCH_FILE_COPIED_SUCCESS));
            }
        }
        else
        {
            // delete fileCopystatus page
            deleteBackupPage();
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(PB_SRCH_FILE_COPY_FAIL));
        }
    }
}

void PlaybackSearch::updateManualBkpStatusEvtAction(QString devName, quint8 percent)
{
    if(devName == m_currDevName)
    {
        if(backupRecords != NULL)
        {
            backupRecords->setPercentage (percent);
        }

        if(backupSingleRecord != NULL)
        {
            backupSingleRecord->setPercentage (percent);
        }
    }
}

void PlaybackSearch::deleteBackupPage()
{
    if(backupRecords != NULL)
    {
        disconnect (backupRecords,
                    SIGNAL(sigBackUpRecsBtnClicked(int)),
                    this,
                    SLOT(slotBackUpRecsHandling(int)));
        delete backupRecords;
        backupRecords = NULL;
    }

    if(backupSingleRecord != NULL)
    {
        disconnect (backupSingleRecord,
                    SIGNAL(sigBackUpSingleRecBtnClicked(int)),
                    this,
                    SLOT(slotBackUpSingleRecHandling(int)));
        delete backupSingleRecord;
        backupSingleRecord = NULL;
    }

    isBackupReqSent = false;
    m_currElement = PB_SEARCH_SEARCH_BTN;
    m_elementList[m_currElement]->forceActiveFocus ();
}

quint8 PlaybackSearch::getCurrPage()
{
    return currPage;
}

void PlaybackSearch::resetOptionSelSateOnPageChange()
{
    if(currPage == selAllOptionOfWhichPage)
    {
        selAllCheckbox->changeState (ON_STATE);
    }
    else
    {
        selAllCheckbox->changeState (OFF_STATE);
    }
    showTableData ();
}

void PlaybackSearch::slotAllOptionSelBtnClick(OPTION_STATE_TYPE_e state, int index)
{
    quint8 actualIndex;
    quint8 recordOnPage;

    if(totalRec < (MAX_REC_IN_ONE_PAGE*(currPage + 1)))
    {
        recordOnPage = totalRec - ((MAX_REC_IN_ONE_PAGE*(currPage)) );
    }
    else
    {
        recordOnPage = MAX_REC_IN_ONE_PAGE;
    }

    if(index == PB_SEARCH_SEL_ALL)
    {
        quint8 tempTick = 0;

        for(quint8 index = 0; index < recordOnPage; index++)
        {
            if(recSelect[index + (MAX_REC_IN_ONE_PAGE *currPage)] == true )
            {
                tempTick ++ ;
            }
        }

        if((((totalTick + recordOnPage) <= MAX_REC_IN_ONE_PAGE))
                || (tempTick == totalTick))
        {
            if(state == ON_STATE)
            {
                selAllOptionOfWhichPage = currPage;
            }
            else
            {
                selAllOptionOfWhichPage = INVALID_FEILD;
            }

            for(quint8 index = 0; index < recordOnPage; index++)
            {
                recSelect[index + (MAX_REC_IN_ONE_PAGE *currPage)] = state;
            }
        }
        else
        {
            if(state == ON_STATE)
            {
                selAllCheckbox->changeState (OFF_STATE);
                infoPage->loadInfoPage (ValidationMessage::getValidationMessage(PB_SRCH_MAX_BKUP_MSG));
            }
            else
            {
                if(selAllOptionOfWhichPage == currPage)
                {
                    state = OFF_STATE;
                    selAllOptionOfWhichPage = INVALID_FEILD;

                    for(quint8 index = 0; index < recordOnPage; index++)
                    {
                        recSelect[index + (MAX_REC_IN_ONE_PAGE *currPage)] = state;
                    }
                }
            }
        }

        for(quint8 index = 0; index < recordOnPage; index++)
        {
            recSelCheckboxes[index]->changeState
                    ((recSelect[index + (MAX_REC_IN_ONE_PAGE *currPage)] == true) ? ON_STATE : OFF_STATE);
        }

        totalTick = 0;

        for(quint8 index = 0; index < MAX_REC_ALLOWED; index++)
        {
            if(recSelect[index] == true )
            {
                totalTick ++ ;
            }
        }
    }
    else
    {
        actualIndex = (currPage*MAX_REC_IN_ONE_PAGE) + ((index - PB_SEARCH_SEL_ALL)/2);

        if(totalTick < MAX_REC_IN_ONE_PAGE)
        {
            if(state == ON_STATE)
            {
                recSelect[actualIndex] = true;
                totalTick++;
            }
            else
            {
                recSelect[actualIndex] = false;
                totalTick--;
            }
        }
        else
        {
            if(state == ON_STATE)
            {
                recSelCheckboxes[((index - PB_SEARCH_SEL_ALL)/2)]->changeState (OFF_STATE);
                state = OFF_STATE;
                infoPage->loadInfoPage (ValidationMessage::getValidationMessage(PB_SRCH_MAX_BKUP_MSG));
            }
            else
            {
                totalTick--;
                state = OFF_STATE;
            }
        }

        recSelect[actualIndex] = state;
        recSelCheckboxes[((index - PB_SEARCH_SEL_ALL)/2)]->changeState (state);

        quint8 tempIndex = 0;

        for(quint8 index = 0; index < recordOnPage; index++)
        {
            if(recSelect[index + (MAX_REC_IN_ONE_PAGE *currPage)] == true )
            {
                tempIndex ++ ;
            }
        }
        selAllCheckbox->changeState ((tempIndex == recordOnPage) ? ON_STATE : OFF_STATE);
    }
}

void PlaybackSearch::slotUpadateCurrentElement (int index)
{
    m_currElement = index;
}

void PlaybackSearch::slotDateChanged()
{
    if((!((PreviousDataCalender.month == startDateCalender->getDDMMYY ()->month)
         && (PreviousDataCalender.year == startDateCalender->getDDMMYY ()->year)))
         || ((PreviousDataCalender.month == startDateCalender->getDDMMYY ()->month) &&
             (startDateCalender->getDDMMYY ()->date > endDataCalender.date)))
    {
        endDateCalender->setDDMMYY(startDateCalender->getDDMMYY ());
    }
    PreviousDataCalender.date = startDateCalender->getDDMMYY ()->date;
    PreviousDataCalender.month = startDateCalender->getDDMMYY ()->month;
    PreviousDataCalender.year = startDateCalender->getDDMMYY ()->year;
    endDataCalender.date = endDateCalender->getDDMMYY()->date;
    endDataCalender.month = endDateCalender->getDDMMYY()->month;
    endDataCalender.year = endDateCalender->getDDMMYY()->year;
}

void PlaybackSearch::slotInfoPageCnfgBtnClick(int)
{
    m_elementList[m_currElement]->forceActiveFocus();
}

void PlaybackSearch::slotBackupBtnClicked(int)
{
   if(isQuickBackupon == false)
   {
    quint8 totalSelRec = 0;
    quint8 recIndex = 0;

    //    if( (m_currDevName != LOCAL_DEVICE_NAME) ||
    //            (UsbControl::showUsbFlag[USB_TYPE_MANUAL] == true) )
    {
        for(quint8 index = 0; index < MAX_REC_ALLOWED; index++)
        {
            if(recSelect[index] == true)
            {
                totalSelRec++;
            }
        }

        if(totalSelRec == 1)
        {
            if(backupSingleRecord == NULL)
            {
                for(recIndex = 0; recIndex < MAX_REC_ALLOWED; recIndex++)
                {
                    if(recSelect[recIndex] == true)
                    {
                        break;
                    }
                }

                backUpSingleRecIndex = recIndex;
                backupSingleRecord = new BackupSingleRecord(&playbackRecData[backUpSingleRecIndex],
                                                            this);

                connect (backupSingleRecord,
                         SIGNAL(sigBackUpSingleRecBtnClicked(int)),
                         this,
                         SLOT(slotBackUpSingleRecHandling(int)));

                getManualBackupLocation();
            }
        }
        else if(totalSelRec > 1)
        {
            if(backupRecords == NULL)
            {
                QStringList startDateTimeList;
                QStringList endDateTimeList;
                QStringList cameraNumberList;

                startDateTimeList.clear ();
                endDateTimeList.clear ();
                cameraNumberList.clear ();

                for(quint8 index = 0; index < MAX_REC_ALLOWED; index++)
                {
                    if(recSelect[index] == true)
                    {
                        QString tempStr = playbackRecData[index].startTime;

                        makeDatetimeInTableFormat(tempStr);
                        startDateTimeList.append (tempStr);

                        tempStr = playbackRecData[index].endTime;
                        makeDatetimeInTableFormat(tempStr);
                        endDateTimeList.append (tempStr);

                        cameraNumberList.append ("Camera" + QString(" %1").arg (playbackRecData[index].camNo));
                    }
                }

                backupRecords = new BackupRecords(startDateTimeList,
                                                  endDateTimeList,
                                                  cameraNumberList,
                                                  this);
                connect (backupRecords,
                         SIGNAL(sigBackUpRecsBtnClicked(int)),
                         this,
                         SLOT(slotBackUpRecsHandling(int)));

                getManualBackupLocation();
            }
        }
        else
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(PB_SRCH_BKUP_MIN_MSG));
        }
    }
    //    else
    //    {
    //        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(PB_SRCH_BKUP_DEV_ERR_MSG));
    //    }
   }
   else
   {
     MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(CMD_BACKUP_IN_PROCESS));
   }
}

void PlaybackSearch::slotBackUpSingleRecHandling (int index)
{
    QString startTime, endTime;
    if(index == BKUP_SINGLE_REC_START_BTN)
    {
        if(backupSingleRecord != NULL)
        {
            backupSingleRecord->getDateTime (startTime, endTime);
            sendBackupCmd (startTime, endTime);
        }
    }
    else
    {
        if(isBackupReqSent)
        {
            // send stop back up cmd
            sendStopBackupCmd();
        }

        if(backupSingleRecord != NULL)
        {
            disconnect (backupSingleRecord,
                        SIGNAL(sigBackUpSingleRecBtnClicked(int)),
                        this,
                        SLOT(slotBackUpSingleRecHandling(int)));
            delete backupSingleRecord;
            backupSingleRecord = NULL;
            m_elementList[m_currElement]->forceActiveFocus ();
        }
    }
}

void PlaybackSearch::slotBackUpRecsHandling(int index)
{
    if(index == BKUP_REC_START_BTN)
    {
        fillMultipleRecBackupCmd ();
        sendMultipleRecBackupCmd();
    }
    else
    {
        if(isBackupReqSent)
        {
            // send stop back up cmd
            sendStopBackupCmd();
        }

        if(backupRecords != NULL)
        {
            disconnect (backupRecords,
                        SIGNAL(sigBackUpRecsBtnClicked(int)),
                        this,
                        SLOT(slotBackUpRecsHandling(int)));
            delete backupRecords;
            backupRecords = NULL;
            m_elementList[m_currElement]->forceActiveFocus ();
        }
    }
}


void PlaybackSearch::slotFileCopyStatusHandling(int)
{
    // send stop back up cmd
    sendStopBackupCmd();
    deleteBackupPage ();
}

bool PlaybackSearch::isCameraRightsAvailable(quint8 camIndex)
{
    bool camRights = false;

    if( (camIndex > 0) && (camIndex <= MAX_CAMERAS))
    {
        for(quint8 index = 0; index < m_cameraList.length(); index++)
        {
            if(m_cameraIndex[index] == camIndex)
            {
                camRights = true;
                break;
            }
        }
    }
    return camRights;
}

void PlaybackSearch::enableDisableControls(bool status)
{
    startDateCalender->setIsEnabled(status);
    endDateCalender->setIsEnabled(status);
    startTimeSpinbox->setIsEnabled(status);
    endTimeSpinbox->setIsEnabled(status);
    searchBtn->setIsEnabled(status);
    channelDropDownBox->setIsEnabled(status);
    recTypeDropDownBox->setIsEnabled(status);
    m_recDriveListDropdown->setIsEnabled(status);
}

void PlaybackSearch::slotSpinboxValueChanged(QString str, quint32 index)
{
    switch(index)
    {
        case PB_SEARCH_DEVICE_SPINBOX:
        {
            str = applController->GetRealDeviceName(str);
            if (str == m_currDevName)
            {
                break;
            }

            bool status = false;
            DEV_TABLE_INFO_t devTableInfo;
            m_currDevName = str;
            channelList.clear ();
            channelList.append("All");

            if(applController->GetDeviceConnectionState (m_currDevName) == CONFLICT)
            {
                QString msgStr = ValidationMessage::getValidationMessage (DEVICE_FIRMWARE_MISMATCH_SERVER_OLD);
                if(applController->GetDeviceInfo (m_currDevName,devTableInfo))
                {
                    if(devTableInfo.deviceConflictType == MX_DEVICE_CONFLICT_TYPE_SERVER_NEW)
                    {
                        msgStr = ValidationMessage::getValidationMessage (DEVICE_FIRMWARE_MISMATCH_SERVER_NEW);
                    }
                }
                infoPage->loadInfoPage (msgStr);
            }
            if(applController->GetTotalCamera (m_currDevName, totalCam))
            {
                status = true;

                QStringList cameraNameList;
                quint8 camIndex[totalCam];

                applController->GetAllCameraNamesOfDevice(m_currDevName,cameraNameList,PLAYBACK_CAM_LIST_TYPE,camIndex);

                for(quint8 index = 0; index < cameraNameList.length(); index++)
                {
                    channelList.append (QString("%1").arg(camIndex[index]) + ": " + cameraNameList.at(index));
                }
            }

            QMap<quint8, QString>  channelMapList;

            for(quint8 index = 0; index < channelList.length (); index++)
            {
                channelMapList.insert (index,channelList.at (index));
            }

            channelDropDownBox->setNewList (channelMapList);

            // if permission is not given then disable and permission given then display only given channel
            enableDisableControls((channelList.length() > 1));

            if(m_currDevName == LOCAL_DEVICE_NAME)
            {
                getDevDateTime ();

                if(!applController->getUserRightsResponseNotify ())
                {
                    m_currDevName = "";
                }
            }
            if((status == true) && (channelList.length() <= 1) && (applController->getUserRightsResponseNotify ()))
            {
                MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(CMD_NO_PRIVILEGE));
            }
        }
        break;

        case PB_SEARCH_REC_TYPE_SPINBOX:
            break;

        case PB_SEARCH_CHANNEL_SPINBOX:
            break;

        case PB_SEARCH_REC_DRIVE_DROPBOX:
            break;
    }
    //    repaint ();
}


void PlaybackSearch::restoreSerchCriteria(QString devName)
{

     bool status = false;
     DEV_TABLE_INFO_t devTableInfo;
     channelList.clear ();
     channelList.append("All");

     devNameDropDownBox->setCurrValue(applController->GetDispDeviceName(devName));
     m_currDevName = devName;

    if(applController->GetDeviceConnectionState (devName) == CONFLICT)
    {
        QString str = ValidationMessage::getValidationMessage (DEVICE_FIRMWARE_MISMATCH_SERVER_OLD);
        if(applController->GetDeviceInfo (devName,devTableInfo))
        {
            if(devTableInfo.deviceConflictType == MX_DEVICE_CONFLICT_TYPE_SERVER_NEW)
            {
                str = ValidationMessage::getValidationMessage (DEVICE_FIRMWARE_MISMATCH_SERVER_NEW);
            }
        }
        infoPage->loadInfoPage (str);
    }
    if(applController->GetTotalCamera (devName, totalCam))
    {
        status = true;

        QStringList cameraNameList;
        quint8 camIndex[totalCam];

        applController->GetAllCameraNamesOfDevice(m_currDevName,cameraNameList,PLAYBACK_CAM_LIST_TYPE,camIndex);

        for(quint8 index = 0; index < cameraNameList.length(); index++)
        {
            channelList.append (QString("%1").arg(camIndex[index]) + ": " + cameraNameList.at(index));
        }
    }

    QMap<quint8, QString>  channelMapList;

    for(quint8 index = 0; index < channelList.length (); index++)
    {
        channelMapList.insert (index,channelList.at (index));
    }

    channelDropDownBox->setNewList (channelMapList);

    // if permission is not given then disable and permission given then display
    //     only given channel
    enableDisableControls((channelList.length() > 1));

    QString channelNo = (tempData.tempChannel == 0) ? ("All") : (QString("%1").arg(channelMapList.value(tempData.tempChannel)));

    recTypeDropDownBox->setCurrValue(recTypeStrMapList.value(tempData.tempType));
    channelDropDownBox->setCurrValue (channelNo);
    m_recDriveListDropdown->setCurrValue (recDriveList[tempData.tempSource]);

    if((status == true) && (channelList.length() <= 1) && (applController->getUserRightsResponseNotify ()))
    {
        MessageBanner::addMessageInBanner(ValidationMessage::getDeviceResponceMessage(CMD_NO_PRIVILEGE));
    }
}

void PlaybackSearch::slotControlBtnclick(int index)
{
    DDMMYY_PARAM_t *dateParam;
    quint32 tempHr, tempMin;
    QDate Date;
    QTime Time;
    QDateTime startDateTime, endDateTime;
    switch(index)
    {
    case PB_SEARCH_SEARCH_BTN:
    {
        selAllCheckbox->changeState (OFF_STATE);
        if(selAllCheckbox->isEnabled ())
            selAllCheckbox->setIsEnabled (false);

        pageNoReadOnly->changeValue ("");
        currPage = 0;
        totalRec = 0;

        clearTableData ();

        dateParam = startDateCalender->getDDMMYY ();
        startTimeSpinbox->currentValue (tempHr, tempMin);

        Date.setDate (dateParam->year, dateParam->month, dateParam->date);
        Time.setHMS (tempHr, tempMin, 0);

        startDateTime.setDate (Date);
        startDateTime.setTime (Time);

        dateParam = endDateCalender->getDDMMYY ();
        endTimeSpinbox->currentValue (tempHr, tempMin);

        Date.setDate (dateParam->year, dateParam->month, dateParam->date);
        Time.setHMS (tempHr, tempMin, 0);
        endDateTime.setDate (Date);
        endDateTime.setTime (Time);

        if(startDateTime >= endDateTime)
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(START_END_DATE_ERR_MSG));
        }
        else if(((endDateTime.toTime_t () - startDateTime.toTime_t ())/(60*60*24)) > 30)
        {
            infoPage->loadInfoPage (ValidationMessage::getValidationMessage(DATE_DIFF_ERR_MSG));
        }
        else
        {
            // For Selecting Proper Camera Search Result
            quint8 playBackDriveType =0;
            QString payloadString;
            SET_COMMAND_e cmdType;
            if(m_recDriveListDropdown->getIndexofCurrElement() == 0)
            {
                cmdType = SEARCH_RCD_ALL_DATA;
                QString channelNumber = channelDropDownBox->getCurrValue();
                QStringList list = channelNumber.split(":");
                quint8 index = 0;
                if(list.length() > 1)
                    index = list.at(0).toUInt();
                payloadLib->setCnfgArrayAtIndex (0, index);
                payloadLib->setCnfgArrayAtIndex (1, recTypeValue[recTypeDropDownBox->getIndexofCurrElement ()]);
                payloadLib->setCnfgArrayAtIndex (2, startDateTime.toString ("ddMMyyyyHHmm"));
                payloadLib->setCnfgArrayAtIndex (3, endDateTime.toString ("ddMMyyyyHHmm"));
                payloadLib->setCnfgArrayAtIndex (4, MAX_REC_ALLOWED);

                payloadString = payloadLib->createDevCmdPayload(5);

                tempData.tempChannel = index;
            }
            else
            {
                playBackDriveType = m_recDriveListDropdown->getIndexofCurrElement() - 1;
                cmdType = SEARCH_RCD_DATA;

                QString channelNumber = channelDropDownBox->getCurrValue();
                QStringList list = channelNumber.split(":");
                quint8 index = 0;
                if(list.length() > 1)
                    index = list.at(0).toUInt();
                payloadLib->setCnfgArrayAtIndex (0, index);
                payloadLib->setCnfgArrayAtIndex (1, recTypeValue[recTypeDropDownBox->getIndexofCurrElement ()]);
                payloadLib->setCnfgArrayAtIndex (2, startDateTime.toString ("ddMMyyyyHHmm"));
                payloadLib->setCnfgArrayAtIndex (3, endDateTime.toString ("ddMMyyyyHHmm"));
                payloadLib->setCnfgArrayAtIndex (4, MAX_REC_ALLOWED);
                payloadLib->setCnfgArrayAtIndex (5, playBackDriveType);

                payloadString = payloadLib->createDevCmdPayload(6);
                tempData.tempChannel = index;
            }

            DevCommParam* param = new DevCommParam();
            param->msgType = MSG_SET_CMD;
            param->cmdType = cmdType;
            param->payload = payloadString;

            m_currDevName = applController->GetRealDeviceName(devNameDropDownBox->getCurrValue());
            processBar->loadProcessBar ();
            applController->processActivity(m_currDevName, DEVICE_COMM, param);
            tempData.tempStartTime = startDateTime;
            tempData.tempEndTime = endDateTime;
        }
    }
        break;

    case PB_SEARCH_FIRST_PAGE:
        currPage = 0;
        resetOptionSelSateOnPageChange();
        break;

    case PB_SEARCH_PREV_PAGE:
        if(currPage)
        {
            currPage--;
        }
        resetOptionSelSateOnPageChange();
        break;

    case PB_SEARCH_NEXT_PAGE:
        currPage++;
        resetOptionSelSateOnPageChange();
        break;

    case PB_SEARCH_LAST_PAGE:
        currPage = totalPage - 1;
        resetOptionSelSateOnPageChange();
        break;

    default:
        // It is play btn click, so emit sig to Layout
        // to start playback
        playbackRecData[(currPage*MAX_REC_IN_ONE_PAGE) +
                ((index - PB_SEARCH_PLAY_1ST)/2)].asyncPbIndex = (currPage*MAX_REC_IN_ONE_PAGE) +
                ((index - PB_SEARCH_PLAY_1ST)/2);

        emit sigPlaybackPlayBtnClick (playbackRecData[(currPage*MAX_REC_IN_ONE_PAGE) +
                ((index - PB_SEARCH_PLAY_1ST)/2)], m_currDevName, true);
        break;
    }
}

void PlaybackSearch::updateDeviceList(void)
{
    QMap<quint8, QString> deviceMapList;
    applController->GetDevNameDropdownMapList(deviceMapList);

    /* Check if selected device found in new updated list or not. It is possible that index of that device name may changed.
     * Hence update device list with current device index */
    for (quint8 deviceIndex = 0; deviceIndex < deviceMapList.count(); deviceIndex++)
    {
        if (devNameDropDownBox->getCurrValue() == deviceMapList.value(deviceIndex))
        {
            devNameDropDownBox->setNewList(deviceMapList, deviceIndex);
            return;
        }
    }

    /* Device name not found in the list. If selected device is not local device then we have to clear the current display search list */
    if (devNameDropDownBox->getIndexofCurrElement() != 0)
    {
        clearTableData();
    }

    /* Now update the list and select the local device */
    devNameDropDownBox->setNewList(deviceMapList, 0);
    slotSpinboxValueChanged(deviceMapList.value(0), PB_SEARCH_DEVICE_SPINBOX);
}
