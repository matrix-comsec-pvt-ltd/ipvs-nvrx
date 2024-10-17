#include "PresetTour.h"
#include "ValidationMessage.h"

#define ELEMENT_HEADING             "Preset Tour"

#define MAX_TABLE_ROW               6
#define MAX_TABLE_SUBELE_ROW        5
#define MAX_PTZ_TOUR_PAGE           4
#define MAX_PATROLL_SPEED           8
#define CUSTOM_TABELCELL_HEIGHT     SCALE_HEIGHT(40)

typedef enum
{
    AUTO_PRESET_TOUR_NAME,
    AUTO_PRESET_POSITIONING_ORDER,
    AUTO_PRESET_PAUSE_BETWEEN_RUNS_IN_SEC,
    AUTO_PRESET_PRESET_POS_ORDER_1,
    AUTO_PRESET_TIME_INTERVAL_ORDER_1,

    PRESET_POSITION_NAME = MAX_AUTO_PRE_TOUR_END_FIELD,

    MANUAL_PRESET_OVERRIDE = (MAX_AUTO_PRE_TOUR_END_FIELD + MAX_PRESET_POS),
    MANUAL_PRESET_TOUR_NUM,
    ACTIVE_TOUR_OVERRIDE,

    TOUR_SCHD_ENTIRE_DAY = (MAX_AUTO_PRE_TOUR_END_FIELD + MAX_PRESET_POS +
    MAX_MANUAL_PRESET_END_FEILDS),

    TOUR_SCHD_PTZ_TOUR_FOR_ENTIRE_DAY,
    TOUR_SCHD_START_TIME1,
    TOUR_SCHD_STOP_TIME1,
    TOUR_SCHD_PTZ_TOUR_FOR_1ST_PERIOD,
    TOUR_SCHD_START_TIME2,
    TOUR_SCHD_STOP_TIME2,
    TOUR_SCHD_PTZ_TOUR_FOR_2ND_PERIOD,

    MAX_PRE_TOUR_FEILDS
}PRE_TOUR_FEILDS_e;

typedef enum
{
    PRE_TOUR_CAM_DROPDOWNBOX,

    PRE_TOUR_MANUAL_PRE_DROPDOWNBOX,
    PRE_TOUR_OVERRIDE_CHECKBOX,
    PRE_TOUR_OVERRIDE_PTZ_TOUR_CHECKBOX,
    PRE_TOUR_WEEKSCHD,
    PRE_TOUR_NUM_DROPDOWNBOX,
    PRE_TOUR_NAME_TXTBX,
    PRE_TOUR_PAUSE_TXTBX,
    PRE_TOUR_ORDER_DROPDOWNBOX,
    PRE_TOUR_PATROLL_SPEED_DROPDOWN,
    PRE_TOUR_VIEW_TIME,
    PRE_TOUR_PREV_BTN = (20 + PRE_TOUR_VIEW_TIME),
    PRE_TOUR_NEXT_BTN,

    MAX_PRE_TOUR_CTRL
}PRE_TOUR_CTRL_e;

static const QString presetTourStrings[] =
{
    "Camera",
    "Manual Preset Tour",
    "Override Scheduled Tour By Manual Tour",
    "Override Active Tour by PTZ Control",
    "Weekly Schedule",
    "Tour Number",
    "Tour Name",
    "Pause between runs",
    "View Order",
    "Patrolling Speed"
};

static const QString rowLabel[] =
{
    "No", "Preset Position", "View Time","Select Preset Position",
    "Previous", "Next"
};

static const QStringList orderList = QStringList() << "Looping" << "Zigzag" << "Random";

PresetTour::PresetTour(QString deviceName, QWidget* parent, DEV_TABLE_INFO_t *devTabInfo)
    : ConfigPageControl(deviceName, parent,MAX_PRE_TOUR_CTRL, devTabInfo),
      currentCameraIndex(0), cnfg2FrmIndx(0), cnfg2ToIndx(0), cnfg3FrmIndx(0),
      cnfg3ToIndx(0), cnfg4FrmIndx(0), cnfg4ToIndx(0), m_currentDropBoxChangeType(0)
{
    entireDaySchdSelect[MAX_PRESET_TOUR_WEEKDAYS - 1] = {0};
    currentPresetPositionIndex[MAX_PRESET_POS_SELECT - 1] = {0};
    ptzTour[MAX_PRESET_POS - 1] = {0};
    reqTourName = false;
    isNextPageSelect = false;
    isCnfgTourReq = false;
    ptzSchd = NULL;
    createDefaultComponents ();
    getCameraList();
    getAllTourName();
}

void PresetTour::createDefaultComponents ()
{
    currentCameraIndex = 0;
    currentPageNo = 0;
    previousPageNo = 0;
    m_currentTourNumber = 0;
    isPTZsupport = false;

    for(quint8 index = 0; index < MAX_PRESET_POS_SELECT; index++)
    {
        currentPresetViewTime.insert(index,"10");
    }

    cameraListDropDownBox = new DropDown((SCALE_WIDTH(PAGE_RIGHT_PANEL_WIDTH) - BGTILE_LARGE_SIZE_WIDTH)/2 + SCALE_WIDTH(5),
                                         SCALE_HEIGHT(20),
                                         BGTILE_LARGE_SIZE_WIDTH,
                                         BGTILE_HEIGHT,
                                         PRE_TOUR_CAM_DROPDOWNBOX,
                                         DROPDOWNBOX_SIZE_320,
                                         presetTourStrings[PRE_TOUR_CAM_DROPDOWNBOX],
                                         cameraList,
                                         this,
                                         "",
                                         false,
                                         SCALE_WIDTH(125));
    m_elementList[PRE_TOUR_CAM_DROPDOWNBOX] = cameraListDropDownBox;
    connect(cameraListDropDownBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (cameraListDropDownBox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotDropDownValueChange(QString,quint32)));

    overrideCheckBox = new OptionSelectButton(cameraListDropDownBox->x () + SCALE_WIDTH(550),
                                              cameraListDropDownBox->y (),
                                              BGTILE_SMALL_SIZE_WIDTH,
                                              BGTILE_HEIGHT,
                                              CHECK_BUTTON_INDEX,
                                              presetTourStrings[PRE_TOUR_OVERRIDE_CHECKBOX],
                                              this,
                                              NO_LAYER,
                                              -1,
                                              MX_OPTION_TEXT_TYPE_SUFFIX,
                                              NORMAL_FONT_SIZE,
                                              PRE_TOUR_OVERRIDE_CHECKBOX);
    m_elementList[PRE_TOUR_OVERRIDE_CHECKBOX] = overrideCheckBox;
    connect(overrideCheckBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    manualPresetTourList.clear ();
    manualPresetTourDropDownBox = new DropDown (cameraListDropDownBox->x (),
                                                cameraListDropDownBox->y () + BGTILE_HEIGHT,
                                                BGTILE_LARGE_SIZE_WIDTH,
                                                BGTILE_HEIGHT,
                                                PRE_TOUR_MANUAL_PRE_DROPDOWNBOX,
                                                DROPDOWNBOX_SIZE_320,
                                                presetTourStrings[PRE_TOUR_MANUAL_PRE_DROPDOWNBOX],
                                                manualPresetTourList,
                                                this,"",
                                                false,
                                                SCALE_WIDTH(35));
    m_elementList[PRE_TOUR_MANUAL_PRE_DROPDOWNBOX] = manualPresetTourDropDownBox;
    connect(manualPresetTourDropDownBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    overrideTourByControlCheckBox = new OptionSelectButton(manualPresetTourDropDownBox->x () + SCALE_WIDTH(550),
                                                           manualPresetTourDropDownBox->y (),
                                                           BGTILE_SMALL_SIZE_WIDTH,
                                                           BGTILE_HEIGHT,
                                                           CHECK_BUTTON_INDEX,
                                                           presetTourStrings[PRE_TOUR_OVERRIDE_PTZ_TOUR_CHECKBOX],
                                                           this,
                                                           NO_LAYER,
                                                           -1,
                                                           MX_OPTION_TEXT_TYPE_SUFFIX,
                                                           NORMAL_FONT_SIZE,
                                                           PRE_TOUR_OVERRIDE_PTZ_TOUR_CHECKBOX);
    m_elementList[PRE_TOUR_OVERRIDE_PTZ_TOUR_CHECKBOX] = overrideTourByControlCheckBox;
    connect(overrideTourByControlCheckBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    weeklySchedule = new PageOpenButton(manualPresetTourDropDownBox->x (),
                                        manualPresetTourDropDownBox->y () +
                                        manualPresetTourDropDownBox->height (),
                                        BGTILE_LARGE_SIZE_WIDTH,
                                        BGTILE_HEIGHT,
                                        PRE_TOUR_WEEKSCHD,
                                        PAGEOPENBUTTON_EXTRALARGE,
                                        presetTourStrings[PRE_TOUR_WEEKSCHD],
                                        this,
                                        "","",
                                        false,
                                        SCALE_WIDTH(760),
                                        COMMON_LAYER,
                                        true,
                                        ALIGN_CENTRE_X_CENTER_Y);
    m_elementList[PRE_TOUR_WEEKSCHD] = weeklySchedule;
    connect(weeklySchedule,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (weeklySchedule,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    elementHeading = new ElementHeading(weeklySchedule->x (),
                                        weeklySchedule->y () + BGTILE_HEIGHT + SCALE_HEIGHT(5),
                                        BGTILE_LARGE_SIZE_WIDTH,
                                        BGTILE_HEIGHT,
                                        ELEMENT_HEADING,
                                        TOP_LAYER,
                                        this,
                                        false,
                                        SCALE_WIDTH(25), NORMAL_FONT_SIZE, true);

    tourNumList.clear ();
    for(quint8 index = 0; index < MAX_PRESET_PER_CAM; index++)
    {
        tourNumList.insert (index,QString("%1").arg(index + 1));
    }

    tourNumDropDownBox = new DropDown(elementHeading->x (),
                                      elementHeading->y () + elementHeading->height (),
                                      BGTILE_LARGE_SIZE_WIDTH,
                                      BGTILE_HEIGHT,
                                      PRE_TOUR_NUM_DROPDOWNBOX,
                                      DROPDOWNBOX_SIZE_90,
                                      presetTourStrings[PRE_TOUR_NUM_DROPDOWNBOX],
                                      tourNumList,
                                      this,
                                      "",
                                      false,
                                      SCALE_WIDTH(90),
                                      MIDDLE_TABLE_LAYER);
    m_elementList[PRE_TOUR_NUM_DROPDOWNBOX] = tourNumDropDownBox;
    connect(tourNumDropDownBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (tourNumDropDownBox,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotDropDownValueChange(QString,quint32)));

    tourNameParam = new TextboxParam();
    tourNameParam->maxChar = 16;
    tourNameParam->labelStr = presetTourStrings[PRE_TOUR_NAME_TXTBX];

    tourNameTextbox = new TextBox(tourNumDropDownBox->x () + SCALE_WIDTH(600),
                                  tourNumDropDownBox->y (),
                                  BGTILE_LARGE_SIZE_WIDTH,
                                  BGTILE_HEIGHT,
                                  PRE_TOUR_NAME_TXTBX,
                                  TEXTBOX_LARGE,
                                  this,
                                  tourNameParam,
                                  NO_LAYER);
    m_elementList[PRE_TOUR_NAME_TXTBX] = tourNameTextbox;
    connect(tourNameTextbox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    pauseTextBoxParam = new TextboxParam();
    pauseTextBoxParam->maxChar = 2;
    pauseTextBoxParam->isNumEntry = true;
    pauseTextBoxParam->minNumValue = 0;
    pauseTextBoxParam->maxNumValue = 60;
    pauseTextBoxParam->validation = QRegExp(QString("[0-9]"));
    pauseTextBoxParam->labelStr = presetTourStrings[PRE_TOUR_PAUSE_TXTBX];
    pauseTextBoxParam->suffixStr = "(0 - 60 min)";
    pauseTextBoxParam->isCentre = false;
    pauseTextBoxParam->leftMargin = SCALE_WIDTH(25);

    pauseTextBox = new TextBox(tourNumDropDownBox->x (),
                               tourNumDropDownBox->y () + BGTILE_HEIGHT,
                               BGTILE_LARGE_SIZE_WIDTH,
                               BGTILE_HEIGHT,
                               PRE_TOUR_PAUSE_TXTBX,
                               TEXTBOX_SMALL,
                               this,
                               pauseTextBoxParam,
                               MIDDLE_TABLE_LAYER);
    m_elementList[PRE_TOUR_PAUSE_TXTBX] = pauseTextBox;
    connect(pauseTextBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    QMap<quint8, QString> viewOrderList;
    for(quint8 index = 0; index < orderList.length (); index++)
    {
        viewOrderList.insert (index,orderList.at (index));
    }
    viewOrderDropDownBox = new DropDown(pauseTextBox->x () + SCALE_WIDTH(600),
                                        pauseTextBox->y (),
                                        BGTILE_LARGE_SIZE_WIDTH,
                                        BGTILE_HEIGHT,
                                        PRE_TOUR_ORDER_DROPDOWNBOX,
                                        DROPDOWNBOX_SIZE_200,
                                        presetTourStrings[PRE_TOUR_ORDER_DROPDOWNBOX],
                                        viewOrderList,
                                        this,
                                        "",
                                        false,
                                        0,
                                        NO_LAYER);
    m_elementList[PRE_TOUR_ORDER_DROPDOWNBOX] = viewOrderDropDownBox;
    connect(viewOrderDropDownBox,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    for(quint8 index = 0 ; index < (MAX_PRESET_TOUR_WEEKDAYS + 1); index++ )
    {
        middleTitle[index] = new BgTile(pauseTextBox->x (),
                                        pauseTextBox->y () + (index)*TABELCELL_HEIGHT + BGTILE_HEIGHT,
                                        BGTILE_LARGE_SIZE_WIDTH,
                                        TABELCELL_HEIGHT,
                                        MIDDLE_LAYER,
                                        this);
    }

    for(quint8 index = 0; index < MAX_TABLE_ROW; index ++ )
    {
        qreal tableCellHeight = ((index == 0) ? CUSTOM_TABELCELL_HEIGHT : TABELCELL_HEIGHT);

        numbers[index] = new TableCell(pauseTextBox->x () + SCALE_WIDTH(10) ,
                                       middleTitle[index]->y () + ((index == 0)?
                                       TABELCELL_HEIGHT : CUSTOM_TABELCELL_HEIGHT),
                                       TABELCELL_ULTRASMALL_SIZE_WIDTH,
                                       tableCellHeight,
                                       this,
                                       index == 0 ? true : false);

        numberLabel[index] = new TextLabel(numbers[index]->x () + (numbers[index]->width ())/2,
                                           numbers[index]->y () + tableCellHeight/2,
                                           NORMAL_FONT_SIZE,
                                           (index == 0 ? rowLabel[0] : QString("%1").arg(index)),
                                           this,
                                           (index == 0 ? HIGHLITED_FONT_COLOR : NORMAL_FONT_COLOR),
                                           NORMAL_FONT_FAMILY,
                                           ALIGN_CENTRE_X_CENTER_Y, 0, 0, (numbers[0]->width() - SCALE_WIDTH(5)));

        presetPosition[index] = new TableCell(numbers[index]->x () + numbers[index]->width (),
                                              numbers[index]->y (),
                                              TABELCELL_EXTRAMEDIUM_SIZE_WIDTH,
                                              tableCellHeight,
                                              this,
                                              index == 0 ? true : false);

        viewTime[index] = new TableCell(presetPosition[index]->x () + presetPosition[index]->width (),
                                        presetPosition[index]->y (),
                                        TABELCELL_SMALL_SIZE_WIDTH,
                                        tableCellHeight,
                                        this,
                                        index == 0 ? true : false);

        numbers[index + MAX_TABLE_ROW] = new TableCell(viewTime[index]->x () + viewTime[index]->width () + SCALE_WIDTH(10),
                                                       viewTime[index]->y (),
                                                       TABELCELL_ULTRASMALL_SIZE_WIDTH,
                                                       tableCellHeight,
                                                       this,
                                                       index == 0 ? true : false);

        numberLabel[index + MAX_TABLE_ROW] = new TextLabel(numbers[index + MAX_TABLE_ROW]->x () + (numbers[index + MAX_TABLE_ROW]->width ())/2,
                                                           numbers[index + 6]->y () + tableCellHeight/2,
                                                           NORMAL_FONT_SIZE,
                                                           (index == 0 ? rowLabel[0] : QString("%1").arg(index + 5)),
                                                           this,
                                                           (index == 0 ? HIGHLITED_FONT_COLOR : NORMAL_FONT_COLOR),
                                                           NORMAL_FONT_FAMILY,
                                                           ALIGN_CENTRE_X_CENTER_Y, 0, 0, (TABELCELL_ULTRASMALL_SIZE_WIDTH - SCALE_WIDTH(5)));

        presetPosition[index + MAX_TABLE_ROW] = new TableCell(numbers[index + MAX_TABLE_ROW]->x () + numbers[index + MAX_TABLE_ROW]->width (),
                                                              numbers[index + MAX_TABLE_ROW]->y (),
                                                              TABELCELL_EXTRAMEDIUM_SIZE_WIDTH,
                                                              tableCellHeight,
                                                              this,
                                                              index == 0 ? true : false);

        viewTime[index + MAX_TABLE_ROW] = new TableCell(presetPosition[index + MAX_TABLE_ROW]->x () + presetPosition[index + MAX_TABLE_ROW]->width (),
                                                        presetPosition[index + 6]->y (),
                                                        TABELCELL_SMALL_SIZE_WIDTH,
                                                        tableCellHeight,
                                                        this,
                                                        index == 0 ? true : false);
    }

    presetPositionLabel[0] = new TextLabel(presetPosition[0]->x () + (presetPosition[0]->width ())/2,
                                           presetPosition[0]->y () + CUSTOM_TABELCELL_HEIGHT/2,
                                           NORMAL_FONT_SIZE,
                                           rowLabel[1],
                                           this,
                                           HIGHLITED_FONT_COLOR,
                                           NORMAL_FONT_FAMILY,
                                           ALIGN_CENTRE_X_CENTER_Y, 0, 0, TABELCELL_EXTRAMEDIUM_SIZE_WIDTH);

    presetPositionLabel[1] = new TextLabel(presetPosition[MAX_TABLE_ROW]->x () + (presetPosition[MAX_TABLE_ROW]->width ())/2,
                                           presetPosition[MAX_TABLE_ROW]->y () + CUSTOM_TABELCELL_HEIGHT/2,
                                           NORMAL_FONT_SIZE,
                                           rowLabel[1],
                                           this,
                                           HIGHLITED_FONT_COLOR,
                                           NORMAL_FONT_FAMILY,
                                           ALIGN_CENTRE_X_CENTER_Y, 0, 0, TABELCELL_EXTRAMEDIUM_SIZE_WIDTH);

    viewTimeLabel[0] = new  TextLabel(viewTime[0]->x () + (viewTime[0]->width ())/2,
                                      viewTime[0]->y () + CUSTOM_TABELCELL_HEIGHT/2,
                                      NORMAL_FONT_SIZE,
                                      Multilang(rowLabel[2].toUtf8().constData()) + "(" + Multilang("sec") + ")",
                                      this,
                                      HIGHLITED_FONT_COLOR,
                                      NORMAL_FONT_FAMILY,
                                      ALIGN_CENTRE_X_CENTER_Y, 0, 0, TABELCELL_SMALL_SIZE_WIDTH);

    viewTimeLabel[1] = new TextLabel(viewTime[6]->x () + (viewTime[6]->width ())/2,
                                     viewTime[6]->y () + CUSTOM_TABELCELL_HEIGHT/2,
                                     NORMAL_FONT_SIZE,
                                     Multilang(rowLabel[2].toUtf8().constData()) + "(" + Multilang("sec") + ")",
                                     this,
                                     HIGHLITED_FONT_COLOR,
                                     NORMAL_FONT_FAMILY,
                                     ALIGN_CENTRE_X_CENTER_Y, 0, 0, TABELCELL_SMALL_SIZE_WIDTH);

    for(quint8 index = 1; index < MAX_TABLE_ROW; index ++ )
    {
        presetPick[index - 1] = new PickList(presetPosition[index]->x () - SCALE_WIDTH(10),
                                             presetPosition[index]->y (),
                                             TABELCELL_EXTRAMEDIUM_SIZE_WIDTH ,
                                             TABELCELL_HEIGHT,
                                             TABELCELL_EXTRAMEDIUM_SIZE_WIDTH - SCALE_WIDTH(45),
                                             SCALE_HEIGHT(30),
                                             "",
                                             presetPositionList,
                                             0,
                                             rowLabel[3],
                                             this,
                                             NO_LAYER,
                                             -1,
                                             PRE_TOUR_VIEW_TIME + (index-1)*2,
                                             true,
                                             false);
        m_elementList[PRE_TOUR_VIEW_TIME + (index-1)*2] =  presetPick[index - 1];
        connect(presetPick[index - 1],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));

        viewTimeTextBoxParam[index -1] = new TextboxParam();
        viewTimeTextBoxParam[index -1]->maxChar = 3;
        viewTimeTextBoxParam[index -1]->isNumEntry = true;
        viewTimeTextBoxParam[index -1]->minNumValue = 5;
        viewTimeTextBoxParam[index -1]->maxNumValue = 255;
        viewTimeTextBoxParam[index -1]->validation = QRegExp(QString("[0-9]"));
        viewTimeTextBoxParam[index -1]->isCentre = false;
        viewTimeTextBoxParam[index -1]->leftMargin = 0;

        viewTimeTextBox[index - 1] = new TextBox(viewTime[index]->x () ,
                                                 viewTime[index]->y (),
                                                 TABELCELL_SMALL_SIZE_WIDTH,
                                                 TABELCELL_HEIGHT,
                                                 PRE_TOUR_VIEW_TIME + (index - 1) *2 +1,
                                                 TEXTBOX_TABLE_TYPE_SMALL,
                                                 this,
                                                 viewTimeTextBoxParam[index -1],
                                                 NO_LAYER);
        m_elementList[PRE_TOUR_VIEW_TIME + (index-1)*2+ 1] =  viewTimeTextBox[index - 1];
        connect( viewTimeTextBox[index - 1],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));

        presetPick[index + 4] = new PickList(presetPosition[index + 6]->x () - SCALE_WIDTH(10),
                                             presetPosition[index + 6]->y (),
                                             TABELCELL_EXTRAMEDIUM_SIZE_WIDTH ,
                                             TABELCELL_HEIGHT,
                                             TABELCELL_EXTRAMEDIUM_SIZE_WIDTH - SCALE_WIDTH(45),
                                             SCALE_HEIGHT(30),
                                             "",
                                             presetPositionList,
                                             0,
                                             rowLabel[3],
                                             this,
                                             NO_LAYER,
                                             -1,
                                             PRE_TOUR_VIEW_TIME +  (index-1)*2+ 10,
                                             true,
                                             false);
         m_elementList[PRE_TOUR_VIEW_TIME + (index-1)*2+ 10] = presetPick[index + 4];
        connect (presetPick[index + 4],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));

        viewTimeTextBoxParam[index + 4] = new TextboxParam();
        viewTimeTextBoxParam[index + 4]->maxChar = 3;
        viewTimeTextBoxParam[index + 4]->isNumEntry = true;
        viewTimeTextBoxParam[index + 4]->minNumValue = 5;
        viewTimeTextBoxParam[index + 4]->maxNumValue = 255;
        viewTimeTextBoxParam[index + 4]->validation = QRegExp(QString("[0-9]"));
        viewTimeTextBoxParam[index + 4]->isCentre = false;
        viewTimeTextBoxParam[index + 4]->leftMargin = 0;

        viewTimeTextBox[index + 4] = new TextBox(viewTime[index + 6]->x () ,
                                                 viewTime[index + 6]->y (),
                                                 TABELCELL_SMALL_SIZE_WIDTH,
                                                 TABELCELL_HEIGHT,
                                                 PRE_TOUR_VIEW_TIME + (index-1)*2+10+1,
                                                 TEXTBOX_TABLE_TYPE_SMALL,
                                                 this,
                                                 viewTimeTextBoxParam[index + 4],
                                                 NO_LAYER);
         m_elementList[PRE_TOUR_VIEW_TIME + (index-1)*2+10+1] = viewTimeTextBox[index + 4];
        connect (viewTimeTextBox[index + 4],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
    }

    prevButton = new ControlButton(PREVIOUS_BUTTON_INDEX,
                                   middleTitle[6]->x (),
                                   middleTitle[6]->y () + CUSTOM_TABELCELL_HEIGHT + SCALE_HEIGHT(5),
                                   BGTILE_LARGE_SIZE_WIDTH,
                                   TABELCELL_HEIGHT,
                                   this,
                                   BOTTOM_LAYER,
                                   SCALE_WIDTH(10),
                                   rowLabel[4],
                                   true,
                                   PRE_TOUR_PREV_BTN,
                                   false);
    m_elementList[PRE_TOUR_PREV_BTN] = prevButton;
    connect (prevButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (prevButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    nextButton = new ControlButton(NEXT_BUTTON_INDEX,
                                   middleTitle[6]->x () + BGTILE_LARGE_SIZE_WIDTH - SCALE_WIDTH(85),
                                   middleTitle[6]->y () + CUSTOM_TABELCELL_HEIGHT + SCALE_HEIGHT(5),
                                   BGTILE_LARGE_SIZE_WIDTH,
                                   TABELCELL_HEIGHT,
                                   this,
                                   NO_LAYER,
                                   SCALE_WIDTH(10),
                                   rowLabel[5],
                                   true,
                                   PRE_TOUR_NEXT_BTN);
    m_elementList[PRE_TOUR_NEXT_BTN] = nextButton;
    connect (nextButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (nextButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    nextButton->setIsEnabled (true);
    prevButton->setIsEnabled (false);

    resetGeometryOfCnfgbuttonRow (SCALE_HEIGHT(20));
}

PresetTour :: ~PresetTour()
{
    disconnect(cameraListDropDownBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect (cameraListDropDownBox,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotDropDownValueChange(QString,quint32)));
    delete cameraListDropDownBox;
    cameraList.clear ();

    disconnect(overrideCheckBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete overrideCheckBox;

    disconnect(manualPresetTourDropDownBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete manualPresetTourDropDownBox;
    manualPresetTourList.clear ();

    disconnect(weeklySchedule,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect (weeklySchedule,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    delete weeklySchedule;

    delete elementHeading;

    disconnect(tourNumDropDownBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect (tourNumDropDownBox,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotDropDownValueChange(QString,quint32)));
    delete tourNumDropDownBox;
    tourNumList.clear ();

    disconnect(tourNameTextbox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete tourNameTextbox;
    delete tourNameParam;

    disconnect(pauseTextBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete pauseTextBox;
    delete pauseTextBoxParam;

    disconnect(viewOrderDropDownBox,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    delete viewOrderDropDownBox;

    for(quint8 index = 0 ; index < (MAX_PRESET_TOUR_WEEKDAYS + 1); index++ )
    {
        delete middleTitle[index];
    }

    for(quint8 index = 0; index < MAX_TABLE_ELEMENTS; index ++ )
    {
        delete numbers[index];
        delete numberLabel[index];
        delete presetPosition[index];
        delete viewTime[index];
    }

    delete presetPositionLabel[0];
    delete presetPositionLabel[1];

    delete viewTimeLabel[0];
    delete viewTimeLabel[1];

    for(quint8 index = 1; index < MAX_TABLE_ROW; index ++ )
    {
        disconnect (presetPick[index - 1],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete presetPick[index -1];

        disconnect (viewTimeTextBox[index - 1],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete viewTimeTextBox[index - 1];
        delete viewTimeTextBoxParam[index -1];

        disconnect (presetPick[index + 4],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete presetPick[index + 4];

        disconnect (viewTimeTextBox[index + 4],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete viewTimeTextBox[index + 4];
        delete viewTimeTextBoxParam[index + 4];
    }

    presetPositionList.clear();

    disconnect (prevButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (prevButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    delete prevButton;

    disconnect (nextButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    disconnect (nextButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    delete nextButton;

    if(ptzSchd != NULL)
    {
        disconnect (ptzSchd,
                    SIGNAL(sigObjectDel()),
                    this,
                    SLOT(slotSubObjectDel()));
        delete ptzSchd;
        ptzSchd = NULL;
    }

    if(overrideTourByControlCheckBox != NULL)
    {
        disconnect (overrideTourByControlCheckBox,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
       DELETE_OBJ(overrideTourByControlCheckBox);
    }
}

void PresetTour::getCameraList ()
{
    cameraList.clear();
    for(quint8 index = 0; index < devTableInfo->totalCams; index++)
    {
        cameraList.insert(index, QString("%1%2%3").arg(index + 1).arg(" : ").arg (applController->GetCameraNameOfDevice(currDevName, index)));
    }
    cameraListDropDownBox->setNewList (cameraList,currentCameraIndex);
}

void PresetTour::getAllTourName ()
{
    reqTourName = true;
    cnfg1FrmIndx = (currentCameraIndex*MAX_PRESET_PER_CAM) + 1;
    cnfg1ToIndx  = (currentCameraIndex*MAX_PRESET_PER_CAM) + MAX_PRESET_PER_CAM;

    QString payloadString = payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                             AUTO_PRESET_TOUR_TABLE_INDEX,
                                                             cnfg1FrmIndx,
                                                             cnfg1ToIndx,
                                                             CNFG_FRM_FIELD,
                                                             CNFG_FRM_FIELD,
                                                             cnfg1ToIndx);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_GET_CFG;
    param->payload = payloadString;
    processBar->loadProcessBar ();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void PresetTour::getConfig1 ()
{
    reqTourName = false;
    createPayload(MSG_GET_CFG);
}

void PresetTour::getConfig()
{
    getCameraList();
    getAllTourName ();
}

void PresetTour::defaultConfig()
{
    createPayload(MSG_DEF_CFG);
}

void PresetTour::saveConfigFeilds ()
{
    quint8 currentPageIndex = (currentPageNo * 10);

    for(quint8 index = 0; index < 10; index++)
    {
        // save Previous Page Values
        currentPresetPositionIndex[currentPageIndex] = presetPick[index]->getCurrentValue ();        
        currentPresetViewTime.replace ((currentPageIndex),viewTimeTextBox[index]->getInputText ());
        currentPageIndex++;
    }

    payloadLib->setCnfgArrayAtIndex(AUTO_PRESET_TOUR_NAME, tourNameTextbox->getInputText());
    payloadLib->setCnfgArrayAtIndex(AUTO_PRESET_POSITIONING_ORDER, viewOrderDropDownBox->getIndexofCurrElement());
    payloadLib->setCnfgArrayAtIndex(AUTO_PRESET_PAUSE_BETWEEN_RUNS_IN_SEC, pauseTextBox->getInputText ());

    for(quint8 index = 0; index < MAX_PRESET_POS_SELECT; index++)
    {
        payloadLib->setCnfgArrayAtIndex((AUTO_PRESET_TIME_INTERVAL_ORDER_1 + index*2), currentPresetViewTime.at (index));
        payloadLib->setCnfgArrayAtIndex((AUTO_PRESET_PRESET_POS_ORDER_1 + (index)*2), currentPresetPositionIndex[index]);
    }

    payloadLib->setCnfgArrayAtIndex ( MANUAL_PRESET_OVERRIDE, overrideCheckBox->getCurrentState ());
    payloadLib->setCnfgArrayAtIndex (MANUAL_PRESET_TOUR_NUM, manualPresetTourDropDownBox->getIndexofCurrElement ());
    payloadLib->setCnfgArrayAtIndex ( ACTIVE_TOUR_OVERRIDE, overrideTourByControlCheckBox->getCurrentState ());

    for(quint8 index = 0; index < MAX_PRESET_TOUR_WEEKDAYS;index++)
    {
        payloadLib->setCnfgArrayAtIndex(TOUR_SCHD_ENTIRE_DAY + (MAX_TOUR_SCHD_END_FEILDS * index), entireDaySchdSelect[index] == true ? 1 : 0);
        payloadLib->setCnfgArrayAtIndex(TOUR_SCHD_START_TIME1 + (MAX_TOUR_SCHD_END_FEILDS * index), startTime1List.at (index));
        payloadLib->setCnfgArrayAtIndex(TOUR_SCHD_STOP_TIME1 + (MAX_TOUR_SCHD_END_FEILDS * index), stopTime1List.at (index));
        payloadLib->setCnfgArrayAtIndex(TOUR_SCHD_START_TIME2 + (MAX_TOUR_SCHD_END_FEILDS * index), startTime2List.at (index));
        payloadLib->setCnfgArrayAtIndex(TOUR_SCHD_STOP_TIME2 + (MAX_TOUR_SCHD_END_FEILDS * index), stopTime2List.at (index));
        payloadLib->setCnfgArrayAtIndex(TOUR_SCHD_PTZ_TOUR_FOR_ENTIRE_DAY + (MAX_TOUR_SCHD_END_FEILDS * index), ptzTour[index*3]);
        payloadLib->setCnfgArrayAtIndex(TOUR_SCHD_PTZ_TOUR_FOR_1ST_PERIOD + (MAX_TOUR_SCHD_END_FEILDS * index), ptzTour[index*3 + 1]);
        payloadLib->setCnfgArrayAtIndex(TOUR_SCHD_PTZ_TOUR_FOR_2ND_PERIOD + (MAX_TOUR_SCHD_END_FEILDS * index), ptzTour[index*3 + 2]);
    }
    reqTourName = false;
}

void PresetTour::saveConfig ()
{
    quint8 tempCount = 0;
    quint8 index;
    quint8 currentPageIndex = (currentPageNo * 10);

    for(quint8 index = 0; index < 10; index++)
    {
        // save Previous Page Values
        currentPresetPositionIndex[currentPageIndex] = presetPick[index]->getCurrentValue ();
        currentPageIndex++;
    }

    for(index = 0; index < MAX_PRESET_POS_SELECT;index++)
    {
        if( currentPresetPositionIndex[index] != 0)
        {
            tempCount++;
        }
    }

    if(tempCount >= 2)
    {
        saveConfigFeilds ();
        createPayload(MSG_SET_CFG);
    }
    else
    {
        infoPage->loadInfoPage (ValidationMessage::getValidationMessage(PRESET_TOUR_POSTN_AGANT_TWO_NM));
        cameraListDropDownBox->setIndexofCurrElement(currentCameraIndex);
        tourNumDropDownBox->setIndexofCurrElement(m_currentTourNumber);
    }
}

void PresetTour::createPayload(REQ_MSG_ID_e msgType)
{
    reqTourName = false;
    isCnfgTourReq = false;

    cnfg1FrmIndx = currentCameraIndex*MAX_PRESET_PER_CAM + 1 + m_currentTourNumber;
    cnfg1ToIndx  = cnfg1FrmIndx;

    cnfg2FrmIndx = currentCameraIndex*MAX_PRESET_POS + 1;
    cnfg2ToIndx  = currentCameraIndex*MAX_PRESET_POS + MAX_PRESET_POS;
    cnfg3ToIndx  = cnfg3FrmIndx = currentCameraIndex + 1;

    cnfg4FrmIndx = currentCameraIndex*7 + 1;
    cnfg4ToIndx  = currentCameraIndex*7 + 7;

    QString payloadString = payloadLib->createDevCnfgPayload(msgType,
                                                             AUTO_PRESET_TOUR_TABLE_INDEX,
                                                             cnfg1FrmIndx,
                                                             cnfg1ToIndx,
                                                             CNFG_FRM_FIELD,
                                                             MAX_AUTO_PRE_TOUR_END_FIELD,
                                                             MAX_AUTO_PRE_TOUR_END_FIELD);
    if (msgType != MSG_DEF_CFG)
    {
        payloadString = payloadLib->createDevCnfgPayload(msgType,
                                                         PRESET_POSITION_TABLE_INDEX,
                                                         cnfg2FrmIndx,
                                                         cnfg2ToIndx,
                                                         CNFG_FRM_FIELD,
                                                         CNFG_FRM_FIELD,
                                                         MAX_PRESET_POS,
                                                         payloadString,
                                                         MAX_AUTO_PRE_TOUR_END_FIELD);
    }

    payloadString = payloadLib->createDevCnfgPayload(msgType,
                                                     MANUAL_PRESET_TOUR_TABLE_INDEX,
                                                     cnfg3FrmIndx,
                                                     cnfg3FrmIndx,
                                                     CNFG_FRM_FIELD,
                                                     MAX_MANUAL_PRESET_END_FEILDS,
                                                     MAX_MANUAL_PRESET_END_FEILDS,
                                                     payloadString,
                                                     (MAX_AUTO_PRE_TOUR_END_FIELD + MAX_PRESET_POS) );

    payloadString = payloadLib->createDevCnfgPayload(msgType,
                                                     TOUR_SCHEDULE_TABLE_INDEX,
                                                     cnfg4FrmIndx,
                                                     cnfg4ToIndx,
                                                     CNFG_FRM_FIELD,
                                                     MAX_TOUR_SCHD_END_FEILDS,
                                                     7*MAX_TOUR_SCHD_END_FEILDS,
                                                     payloadString,
                                                     (MAX_AUTO_PRE_TOUR_END_FIELD + MAX_PRESET_POS +
                                                      MAX_MANUAL_PRESET_END_FEILDS));
    DevCommParam* param = new DevCommParam();
    param->msgType = msgType;
    param->payload = payloadString;
    processBar->loadProcessBar ();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void PresetTour::sendCommand(SET_COMMAND_e cmdType, quint8 totalfeilds)
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = cmdType;
    param->payload = payloadLib->createDevCmdPayload(totalfeilds);
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void PresetTour::getConfigOfPresetTour ()
{
    reqTourName = false;
    isCnfgTourReq = true;

    cnfg1FrmIndx = currentCameraIndex*MAX_PRESET_PER_CAM + m_currentTourNumber + 1;
    cnfg1ToIndx  = cnfg1FrmIndx;

    cnfg2FrmIndx = currentCameraIndex*MAX_PRESET_POS + 1;
    cnfg2ToIndx  = currentCameraIndex*MAX_PRESET_POS + MAX_PRESET_POS;

    QString payloadString = payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                             AUTO_PRESET_TOUR_TABLE_INDEX,
                                                             cnfg1FrmIndx,
                                                             cnfg1ToIndx,
                                                             CNFG_FRM_FIELD,
                                                             MAX_AUTO_PRE_TOUR_END_FIELD,
                                                             MAX_AUTO_PRE_TOUR_END_FIELD);

    payloadString = payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                     PRESET_POSITION_TABLE_INDEX,
                                                     cnfg2FrmIndx,
                                                     cnfg2ToIndx,
                                                     CNFG_FRM_FIELD,
                                                     CNFG_FRM_FIELD,
                                                     MAX_PRESET_POS,
                                                     payloadString,
                                                     MAX_AUTO_PRE_TOUR_END_FIELD);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_GET_CFG;
    param->payload = payloadString;
    processBar->loadProcessBar ();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void PresetTour::processDeviceResponse (DevCommParam *param, QString deviceName)
{
    bool isUnloadProcessbar = true;

    if (deviceName != currDevName)
    {
        processBar->unloadProcessBar();
        return;
    }

    switch(param->deviceStatus)
    {
        case CMD_SUCCESS:
        {
            switch(param->msgType)
            {
                case MSG_GET_CFG:
                {
                    payloadLib->parsePayload (param->msgType, param->payload);
                    if(payloadLib->getcnfgTableIndex (0) == AUTO_PRESET_TOUR_TABLE_INDEX)
                    {
                        if(reqTourName == true)
                        {
                            manualPresetTourList.clear ();
                            manualPresetTourList.insert (0,"None");
                            for(quint8 index = 0; index < MAX_TABLE_SUBELE_ROW; index++)
                            {
                                QString tempStr = (payloadLib->getCnfgArrayAtIndex(index)).toString () ;
                                if(tempStr != "")
                                {
                                    manualPresetTourList.insert(index + 1, QString("%1%2%3").arg(index + 1) .arg(" : ").arg (tempStr));
                                }
                            }

                            isUnloadProcessbar = false;
                            getConfig1 ();
                        }
                        else
                        {
                            QString tempStr;
                            quint8 tempInt;

                            tempStr = payloadLib->getCnfgArrayAtIndex (AUTO_PRESET_TOUR_NAME).toString ();
                            tourNameTextbox->setInputText(tempStr);
                            m_configResponse[AUTO_PRESET_TOUR_NAME] = tempStr;

                            viewOrderDropDownBox->setIndexofCurrElement(payloadLib->getCnfgArrayAtIndex (AUTO_PRESET_POSITIONING_ORDER).toUInt ());
                            m_configResponse[AUTO_PRESET_POSITIONING_ORDER] = payloadLib->getCnfgArrayAtIndex(AUTO_PRESET_POSITIONING_ORDER);

                            tempStr = payloadLib->getCnfgArrayAtIndex (AUTO_PRESET_PAUSE_BETWEEN_RUNS_IN_SEC).toString ();
                            pauseTextBox->setInputText (tempStr);
                            m_configResponse[AUTO_PRESET_PAUSE_BETWEEN_RUNS_IN_SEC] = tempStr;

                            currentPresetViewTime.clear ();
                            memset(&currentPresetPositionIndex,0,sizeof(currentPresetPositionIndex));
                            for(quint8 index = 0; index < MAX_PRESET_POS_SELECT; index++)
                            {
                                tempInt = payloadLib->getCnfgArrayAtIndex (AUTO_PRESET_PRESET_POS_ORDER_1 + index*2).toUInt ();
                                currentPresetPositionIndex[index] = tempInt;
                                m_configResponse[(AUTO_PRESET_PRESET_POS_ORDER_1 + index*2)] = tempInt;

                                tempStr = payloadLib->getCnfgArrayAtIndex (AUTO_PRESET_TIME_INTERVAL_ORDER_1 + index*2).toString ();
                                currentPresetViewTime.insert (index,tempStr);
                                m_configResponse[(AUTO_PRESET_TIME_INTERVAL_ORDER_1 + index*2)] = tempStr;
                            }
                        }
                    }

                    if(payloadLib->getcnfgTableIndex (1) == PRESET_POSITION_TABLE_INDEX)
                    {
                        presetPositionList.clear ();
                        presetPositionList.insert(0, "None");

                        for(quint8 index = 0; index < MAX_PRESET_POS; index++)
                        {
                            QString tempStr = "";
                            tempStr = payloadLib->getCnfgArrayAtIndex(PRESET_POSITION_NAME + index).toString();
                            presetPositionList.insert((index+1), tempStr);
                            m_configResponse[PRESET_POSITION_NAME + index] = tempStr;
                        }

                        for(quint8 index = 0; index < MAX_TABLE_SUBELE_ROW; index++)
                        {
                            presetPick[index]->changeOptionList(presetPositionList, currentPresetPositionIndex[index]);
                            presetPick[index + MAX_TABLE_SUBELE_ROW]->changeOptionList(presetPositionList, currentPresetPositionIndex[index + 5]);
                        }

                        quint8 currentPageIndex = (currentPageNo * 10) + 1;

                        for(quint8 index = 1; index < MAX_TABLE_ROW; index++ )
                        {
                            numberLabel[index]->changeText (QString("%1").arg (currentPageIndex));
                            numberLabel[index + 6]->changeText (QString("%1").arg (currentPageIndex + 5));
                            currentPageIndex++;
                        }

                        currentPageIndex = (currentPageNo * 10);
                        for(quint8 index = 0; index < 10; index++)
                        {
                            // asign New Page Value
                            presetPick[index]->changeValue (currentPresetPositionIndex[currentPageIndex]);
                            viewTimeTextBox[index]->setInputText (currentPresetViewTime.at (currentPageIndex++));
                        }
                    }

                    if(payloadLib->getcnfgTableIndex (2) == MANUAL_PRESET_TOUR_TABLE_INDEX)
                    {
                        quint8 temp;
                        temp = payloadLib->getCnfgArrayAtIndex ( MANUAL_PRESET_OVERRIDE ).toUInt ();
                        overrideCheckBox->changeState (temp == 1 ? ON_STATE : OFF_STATE);
                        m_configResponse[MANUAL_PRESET_OVERRIDE] = temp;

                        temp = payloadLib->getCnfgArrayAtIndex ( MANUAL_PRESET_TOUR_NUM ).toUInt ();
                        manualPresetTourDropDownBox->setNewList (manualPresetTourList,temp);
                        m_configResponse[MANUAL_PRESET_TOUR_NUM] = temp;

                        temp = payloadLib->getCnfgArrayAtIndex ( ACTIVE_TOUR_OVERRIDE ).toUInt ();
                        overrideTourByControlCheckBox->changeState (temp == 1 ? ON_STATE : OFF_STATE);
                        m_configResponse[ACTIVE_TOUR_OVERRIDE] = temp;
                    }

                    if(payloadLib->getcnfgTableIndex (3) == TOUR_SCHEDULE_TABLE_INDEX)
                    {
                        QString time;

                        startTime1List.clear ();
                        startTime2List.clear ();
                        stopTime1List.clear ();
                        stopTime2List.clear ();

                        memset(&ptzTour, 0, sizeof(ptzTour));
                        for(quint8 index = 0; index < MAX_PRESET_TOUR_WEEKDAYS; index++)
                        {
                            entireDaySchdSelect[index] = payloadLib->getCnfgArrayAtIndex(TOUR_SCHD_ENTIRE_DAY + (MAX_TOUR_SCHD_END_FEILDS * index)).toBool ();
                            m_configResponse[TOUR_SCHD_ENTIRE_DAY + (MAX_TOUR_SCHD_END_FEILDS * index)] = entireDaySchdSelect[index];

                            time = payloadLib->getCnfgArrayAtIndex(TOUR_SCHD_START_TIME1 + (MAX_TOUR_SCHD_END_FEILDS * index)).toString ();
                            startTime1List.append (time);
                            m_configResponse[TOUR_SCHD_START_TIME1 + (MAX_TOUR_SCHD_END_FEILDS * index)] = time;

                            time = payloadLib->getCnfgArrayAtIndex(TOUR_SCHD_STOP_TIME1 + (MAX_TOUR_SCHD_END_FEILDS * index)).toString ();
                            stopTime1List.append (time);
                            m_configResponse[TOUR_SCHD_STOP_TIME1 + (MAX_TOUR_SCHD_END_FEILDS * index)] = time;

                            time = payloadLib->getCnfgArrayAtIndex(TOUR_SCHD_START_TIME2 + (MAX_TOUR_SCHD_END_FEILDS * index)).toString ();
                            startTime2List.append (time);
                            m_configResponse[TOUR_SCHD_START_TIME2 + (MAX_TOUR_SCHD_END_FEILDS * index)] = time;

                            time = payloadLib->getCnfgArrayAtIndex (TOUR_SCHD_STOP_TIME2 + (MAX_TOUR_SCHD_END_FEILDS * index)).toString ();
                            stopTime2List.append (time);
                            m_configResponse[TOUR_SCHD_STOP_TIME2 + (MAX_TOUR_SCHD_END_FEILDS * index)] = time;

                            ptzTour[(index*3)] = payloadLib->getCnfgArrayAtIndex(TOUR_SCHD_PTZ_TOUR_FOR_ENTIRE_DAY + (MAX_TOUR_SCHD_END_FEILDS * index)).toUInt ();
                            m_configResponse[TOUR_SCHD_PTZ_TOUR_FOR_ENTIRE_DAY + (MAX_TOUR_SCHD_END_FEILDS * index)] = ptzTour[(index*3)];

                            ptzTour[(index*3) + 1] = payloadLib->getCnfgArrayAtIndex(TOUR_SCHD_PTZ_TOUR_FOR_1ST_PERIOD + (MAX_TOUR_SCHD_END_FEILDS * index)).toUInt ();
                            m_configResponse[TOUR_SCHD_PTZ_TOUR_FOR_1ST_PERIOD + (MAX_TOUR_SCHD_END_FEILDS * index)] = ptzTour[(index*3) + 1];

                            ptzTour[(index*3) + 2] = payloadLib->getCnfgArrayAtIndex(TOUR_SCHD_PTZ_TOUR_FOR_2ND_PERIOD + (MAX_TOUR_SCHD_END_FEILDS * index)).toUInt ();
                            m_configResponse[TOUR_SCHD_PTZ_TOUR_FOR_2ND_PERIOD + (MAX_TOUR_SCHD_END_FEILDS * index)] = ptzTour[(index*3) +2];
                        }
                    }
                }
                break;

                case MSG_SET_CFG:
                {
                    isUnloadProcessbar = false;
                    MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
                    currentCameraIndex = cameraListDropDownBox->getIndexofCurrElement ();
                    m_currentTourNumber = tourNumDropDownBox->getIndexofCurrElement();
                    getConfig ();
                }
                break;

                case MSG_DEF_CFG:
                {
                    isUnloadProcessbar = false;
                    currentCameraIndex = cameraListDropDownBox->getIndexofCurrElement ();
                    m_currentTourNumber = tourNumDropDownBox->getIndexofCurrElement();
                    getConfig ();
                }
                break;

                default:
                {
                    /* Nothing to do */
                }
                break;
            }
        }
        break;

        default:
        {
            infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
            cameraListDropDownBox->setIndexofCurrElement (currentCameraIndex);
        }
        break;
    }

    update ();
    if(isUnloadProcessbar)
    {
        processBar->unloadProcessBar ();
    }
}

void PresetTour::fillPresetPosition(quint8 pageNumber)
{
    quint8 currentPageIndex = (pageNumber * 10) + 1;
    quint8 prevPageIndex = (previousPageNo * 10);
    previousPageNo = pageNumber;

    for(quint8 index = 1; index < MAX_TABLE_ROW; index++ )
    {
        numberLabel[index]->changeText (QString("%1").arg (currentPageIndex));
        numberLabel[index + 6]->changeText (QString("%1").arg (currentPageIndex + 5));
        currentPageIndex++;
    }

    currentPageIndex = (pageNumber * 10);

    for(quint8 index = 0; index < 10; index++)
    {
        // save Previous Page Values
        currentPresetPositionIndex[prevPageIndex] = presetPick[index]->getCurrentValue ();
        currentPresetViewTime.replace ((prevPageIndex),viewTimeTextBox[index]->getInputText ());

        // assign New Page Value
        presetPick[index]->changeValue (currentPresetPositionIndex[currentPageIndex]);
        viewTimeTextBox[index]->setInputText (currentPresetViewTime.at (currentPageIndex++));

        prevPageIndex++;
    }

    if(currentPageNo != 0)
    {
        prevButton->setIsEnabled (true);
        m_elementList[m_currentElement]->forceActiveFocus ();
    }
    else
    {
        if((m_currentElement == PRE_TOUR_NEXT_BTN) || (m_currentElement == PRE_TOUR_PREV_BTN))
        {
            m_currentElement = PRE_TOUR_NEXT_BTN;
            m_elementList[m_currentElement]->forceActiveFocus ();
        }
        prevButton->setIsEnabled (false);
    }

    if(currentPageNo == (MAX_PTZ_TOUR_PAGE-1))
    {
        if(m_currentElement == PRE_TOUR_NEXT_BTN)
        {
            m_currentElement = PRE_TOUR_PREV_BTN;
            m_elementList[m_currentElement]->forceActiveFocus ();
        }
        nextButton->setIsEnabled (false);
    }
    else
    {
        nextButton->setIsEnabled (true);
        m_elementList[m_currentElement]->forceActiveFocus ();
    }
}

bool PresetTour::isUserChangeConfig()
{
    bool isChange = false;

    do
    {
        if(m_configResponse.isEmpty())
        {
            break;
        }

        if(m_configResponse[AUTO_PRESET_TOUR_NAME] !=  tourNameTextbox->getInputText())
        {
            isChange = true;
            break;
        }
        if(m_configResponse[AUTO_PRESET_POSITIONING_ORDER] != viewOrderDropDownBox->getIndexofCurrElement())
        {
            isChange = true;
            break;
        }
        if(m_configResponse[AUTO_PRESET_PAUSE_BETWEEN_RUNS_IN_SEC] !=  pauseTextBox->getInputText ())
        {
            isChange = true;
            break;
        }

        quint8 currentPageIndex = (currentPageNo * 10);

        for(quint8 index = 0; index < 10; index++)
        {
            // save Previous Page Values
            currentPresetPositionIndex[currentPageIndex] = presetPick[index]->getCurrentValue ();
            currentPresetViewTime.replace ((currentPageIndex),viewTimeTextBox[index]->getInputText ());
            currentPageIndex++;
        }

        quint8 index = 0;
        quint16 temp = 0;
        for( ; index < MAX_PRESET_POS_SELECT; index++)
        {
            temp = (AUTO_PRESET_TIME_INTERVAL_ORDER_1 + index*2);
            if(m_configResponse[temp] != currentPresetViewTime.at (index))
            {
                isChange = true;
                break;
            }

            temp = (AUTO_PRESET_PRESET_POS_ORDER_1 + index*2);
            if(m_configResponse[temp] != currentPresetPositionIndex[index])
            {
                isChange = true;
                break;
            }
        }

        if( index < MAX_PRESET_POS_SELECT)
        {
            break;
        }

        if(m_configResponse[MANUAL_PRESET_OVERRIDE] != overrideCheckBox->getCurrentState ())
        {
            isChange = true;
            break;
        }
        if(m_configResponse[MANUAL_PRESET_TOUR_NUM] != manualPresetTourDropDownBox->getIndexofCurrElement ())
        {
            isChange = true;
            break;
        }
        if(m_configResponse[ACTIVE_TOUR_OVERRIDE] != overrideTourByControlCheckBox->getCurrentState())
        {
            isChange = true;
            break;
        }

        for(index = 0; index < MAX_PRESET_TOUR_WEEKDAYS;index++)
        {
            temp = (TOUR_SCHD_ENTIRE_DAY + (MAX_TOUR_SCHD_END_FEILDS * index));
            if(m_configResponse[temp] != entireDaySchdSelect[index])
            {
                isChange = true;
                break;
            }

            temp = (TOUR_SCHD_START_TIME1 + (MAX_TOUR_SCHD_END_FEILDS * index));
            if(m_configResponse[temp] != startTime1List.at (index))
            {
                isChange = true;
                break;
            }

            temp = (TOUR_SCHD_STOP_TIME1 + (MAX_TOUR_SCHD_END_FEILDS * index));
            if(m_configResponse[temp] != stopTime1List.at (index))
            {
                isChange = true;
                break;
            }

            temp = (TOUR_SCHD_START_TIME2 + (MAX_TOUR_SCHD_END_FEILDS * index));
            if(m_configResponse[temp] != startTime2List.at (index))
            {
                isChange = true;
                break;
            }

            temp = (TOUR_SCHD_STOP_TIME2 + (MAX_TOUR_SCHD_END_FEILDS * index));
            if(m_configResponse[temp] != stopTime2List.at (index))
            {
                isChange = true;
                break;
            }

            temp = (TOUR_SCHD_START_TIME1 + (MAX_TOUR_SCHD_END_FEILDS * index));
            if(m_configResponse[temp] != stopTime2List.at (index))
            {
                isChange = true;
                break;
            }

            temp = (TOUR_SCHD_PTZ_TOUR_FOR_ENTIRE_DAY + (MAX_TOUR_SCHD_END_FEILDS * index));
            if(m_configResponse[temp] != ptzTour[index*3])
            {
                isChange = true;
                break;
            }

            temp = (TOUR_SCHD_PTZ_TOUR_FOR_1ST_PERIOD + (MAX_TOUR_SCHD_END_FEILDS * index));
            if(m_configResponse[temp] !=  ptzTour[index*3 + 1])
            {
                isChange = true;
                break;
            }

            temp = (TOUR_SCHD_PTZ_TOUR_FOR_2ND_PERIOD + (MAX_TOUR_SCHD_END_FEILDS * index));
            if(m_configResponse[temp] !=  ptzTour[index*3 + 2])
            {
                isChange = true;
                break;
            }
        }

        if( index < MAX_PRESET_TOUR_WEEKDAYS)
        {
            break;
        }

    }while(0);

    return isChange;
}

void PresetTour::handleInfoPageMessage(int index)
{
    if(index == INFO_OK_BTN)
    {
        if(infoPage->getText() == (ValidationMessage::getValidationMessage(SAVE_CHANGES)))
        {
            saveConfig();
        }
    }
    else
    {
        if(infoPage->getText() == (ValidationMessage::getValidationMessage(SAVE_CHANGES)))
        {
            currentCameraIndex = cameraListDropDownBox->getIndexofCurrElement ();
            switch(m_currentDropBoxChangeType)
            {
                case PRE_TOUR_CAM_DROPDOWNBOX:
                    getConfig();
                    break;

                case PRE_TOUR_NUM_DROPDOWNBOX:
                    m_currentTourNumber = tourNumDropDownBox->getIndexofCurrElement();
                    getConfigOfPresetTour();
                    break;

                default:
                    break;
            }
        }
    }
}

void PresetTour::slotButtonClick(int index)
{
    switch(index)
    {
        case PRE_TOUR_PREV_BTN:
        {
            if(currentPageNo > 0)
            {
                currentPageNo --;
            }
            fillPresetPosition(currentPageNo);
        }
        break;

        case PRE_TOUR_NEXT_BTN:
        {
            if (currentPageNo != (MAX_PTZ_TOUR_PAGE - 1))
            {
                currentPageNo ++;
            }
            fillPresetPosition(currentPageNo);
        }
        break;

        case PRE_TOUR_WEEKSCHD:
        {
            if(ptzSchd == NULL)
            {
                ptzSchd = new PTZSchd(manualPresetTourList,
                                      entireDaySchdSelect,
                                      startTime1List,
                                      stopTime1List,
                                      startTime2List,
                                      stopTime2List,
                                      ptzTour,
                                      parentWidget ());
                connect (ptzSchd,
                         SIGNAL(sigObjectDel()),
                         this,
                         SLOT(slotSubObjectDel()));
            }
        }
        break;

        default:
            break;
    }//Switch Case

    update ();
}

void PresetTour::slotSubObjectDel()
{
    if(ptzSchd != NULL)
    {
        disconnect (ptzSchd,
                    SIGNAL(sigObjectDel()),
                    this,
                    SLOT(slotSubObjectDel()));
        delete ptzSchd;
        ptzSchd = NULL;
    }
    m_elementList[m_currentElement]->forceActiveFocus ();
}

void PresetTour::slotDropDownValueChange (QString , quint32 index)
{
    m_currentDropBoxChangeType = index;
    switch(index)
    {
        case PRE_TOUR_CAM_DROPDOWNBOX:
            if(isUserChangeConfig())
            {
                infoPage->loadInfoPage(ValidationMessage::getValidationMessage(SAVE_CHANGES),true,false,"",CONFORMATION_BTN_YES,CONFORMATION_BTN_NO);
            }
            else
            {
                currentCameraIndex = cameraListDropDownBox->getIndexofCurrElement ();
                getConfig ();
            }
            break;

        case PRE_TOUR_NUM_DROPDOWNBOX:
            if(isUserChangeConfig())
            {
                infoPage->loadInfoPage(ValidationMessage::getValidationMessage(SAVE_CHANGES),true,false,"",CONFORMATION_BTN_YES,CONFORMATION_BTN_NO);
            }
            else
            {
                m_currentTourNumber = tourNumDropDownBox->getIndexofCurrElement();
                getConfigOfPresetTour();
            }
            break;

        default:
            break;
    }
}
