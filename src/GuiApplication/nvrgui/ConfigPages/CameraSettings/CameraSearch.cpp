#include "CameraSearch.h"
#include "ValidationMessage.h"
#include "MxCommandFields.h"

#define CAM_SERCH_FEILD_CELL_HEIGHT         SCALE_HEIGHT(60)
#define CAM_SERCH_BGTILE_WIDTH              SCALE_WIDTH(979)
#define CAM_ADD_NO                          0
#define MAX_NAVIGATE_CONTROL_IN_ROW         3

#define CAMERA_SETTING_FROM_FIELD           1
#define CAMERA_SETTING_TO_FIELD_IP_TABLE    16
#define CNFG_TO_INDEX                       1
#define CAM_SEARCH_AUTO_CNFG_FIELD          20

#define DEFAULT_FIELD_VALUE_STR             "-"

typedef enum
{
    CAM_SRCH_SRCH_FILTER_DROPBOX,
    CAM_SRCH_AUTO_ADD,
    CAM_SRCH_SEL_ALL_CAM,
    CAM_SRCH_SEL_CAM,
    CAM_SRCH_ADD_CAM,
    CAM_SRCH_TEST_CAM,
    CAM_SRCH_PREVIOUS_BUTTON = (CAM_SRCH_SEL_ALL_CAM + 3*MAX_RECORD_DATA),
    CAM_SRCH_SEARCH_BUTTON,
    CAM_SRCH_CANCEL_BUTTON,
    CAM_SRCH_NEXT_BUTTON,
    CAM_SRCH_FAIL_REPORT_BUTTON,
    CAM_SRCH_ADVANCE_BUTTON,
    CAM_SRCH_AUTO_ADD_CAM_BUTTON,
    MAX_CAM_SRCH_ELEMENT
}CAM_SRCH_ELEMENT_e;

typedef enum
{
    FAILURE_IP_ADDR,
    FAILURE_REASON,
    MAX_FAILURE_RESPOSE
}FAILURE_RESPOSE_e;

static const QString cameraSearchStr[] =
{
    "Auto Configure",
    "IP Address",
    "HTTP Port",
    "Brand",
    "Model",
    "ONVIF Port",
    "Add",
    "Test",
    "Previous",
    "Search",
    "Stop",
    "Next"
};

static const QStringList searchFilterList = QStringList() << "All"
                                                          << "Added"
                                                          << "Not Added";

static const QString failureStr[] = { "Authentication Failure",
                                      "Camera Unavailable",
                                      "Communication Error"};

CameraSearch::CameraSearch(QString deviceName, QWidget* parent, DEV_TABLE_INFO_t *devTabInfo)
    : ConfigPageControl(deviceName, parent,MAX_CAM_SRCH_ELEMENT, devTabInfo,MAX_CNFG_BTN_TYPE),
      isUpdateBeforeSearch(0), maximumPages(0), maxSearchListCount(0)
{
    m_autoAddCamera = NULL;
    isBlockListFlag = false;
    advanceCameraSearch = NULL;
    requestCameraParam.clear();
    clearAllList();
    createDefaultComponent();
    CameraSearch::getConfig();
    getAcqListTimer = new QTimer(this);
    connect (getAcqListTimer,
             SIGNAL(timeout()),
             this,
             SLOT(slotGetAcqListTimerTimeout()));
    getAcqListTimer->setInterval(3000);
    getAcqListTimer->setSingleShot(true);

    isAdvanceCameraSearchRunning = false;
    this->show();
}

CameraSearch::~CameraSearch()
{
    cancelCamAutoSearchCmd();
    delete topBgtile;
    delete bottomBgtile;

    disconnect (searchTypeDropDown,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotFilterValueChanged(QString,quint32)));
    disconnect (searchTypeDropDown,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete searchTypeDropDown;

    if(autoAddIconTimer->isActive())
    {
        autoAddIconTimer->stop();
    }
    delete autoAddIconTimer;

    if(IS_VALID_OBJ(autoAddButton))
    {
        disconnect (autoAddButton,
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotButtonClick(int)));
        disconnect (autoAddButton,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(autoAddButton);
    }

    disconnect (selectAllCam,
                SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                this,
                SLOT(slotOptionSelectionButton(OPTION_STATE_TYPE_e,int)));
    disconnect (selectAllCam,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete selectAllCam;

    disconnect(failReportButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect (failReportButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    delete failReportButton;

    disconnect(advnaceSearchButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect (advnaceSearchButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    delete advnaceSearchButton;

    for(quint8 index = 0; index < MAX_RECORD_FEILDS; index++)
    {
        delete fieldsHeading[index];
        if(index < (MAX_RECORD_FEILDS -1))
        {
            delete elided[index];
            delete elementHeading[index];
        }
    }

    for(quint8 index = 0; index < MAX_RECORD_DATA; index++)
    {

        delete srNumber[index];
        delete ipAddressCell[index];
        delete ipv4AddressStr[index];
        delete ipv6AddressStr[index];
        delete httpPorts[index];
        delete httpPortsStr[index];
        delete brands[index];
        delete brandsStr[index];
        delete model[index];
        delete modelStr[index];
        delete onvifPorts[index];
        delete onvifPortStr[index];
        delete addCameraCell[index];

        disconnect (selectCam[index],
                    SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                    this,
                    SLOT(slotOptionSelectionButton(OPTION_STATE_TYPE_e,int)));
        disconnect (selectCam[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete selectCam[index];

        disconnect(addCameraBtn[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect (addCameraBtn[index],
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotAddButtonClick(int)));
        delete addCameraBtn[index] ;

        disconnect(testCameraBtn[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpdateCurrentElement(int)));
        disconnect (testCameraBtn[index],
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotTestButtonClick(int)));
        delete testCameraBtn[index];
        delete testCamera[index];
    }

    disconnect(previousButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect (previousButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    delete previousButton;

    disconnect(searchButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect (searchButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    delete searchButton;

    disconnect(stopButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect (stopButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    delete stopButton;

    disconnect(nextButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));
    disconnect (nextButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    delete nextButton;

    if(addCamera != NULL)
    {
        disconnect (addCamera,
                    SIGNAL(sigDeleteObject(quint8,bool,QString,QString,
                                           QString,bool,QString,QString,
                                           QString,QString,QString,quint8,quint8)),
                    this,
                    SLOT(slotAddCameraDelete(quint8,bool,QString,QString,
                                             QString,bool,QString,QString,
                                             String,QString,QString,quint8,quint8)));
        DELETE_OBJ(addCamera);
    }

    DELETE_OBJ(cameraSearchProcess);
    DELETE_OBJ(searchRunningInfo);

    if(testCam != NULL)
    {
        disconnect (testCam,
                    SIGNAL(sigCreateCMDRequest(SET_COMMAND_e,quint8)),
                    this,
                    SLOT(slotCreateCMDRequest(SET_COMMAND_e,quint8)));
        disconnect (testCam,
                    SIGNAL(sigDeleteObject(quint8)),
                    this,
                    SLOT(slotTestCamDelete(quint8)));
        DELETE_OBJ(testCam);
    }

    if(advanceCameraSearch != NULL)
    {
        disconnect (advanceCameraSearch,
                    SIGNAL(sigCreateCMDRequest(SET_COMMAND_e,quint8)),
                    this,
                    SLOT(slotCreateCMDRequest (SET_COMMAND_e,quint8)));
        disconnect (advanceCameraSearch,
                    SIGNAL(sigObjectDelete(bool)),
                    this,
                    SLOT(slotAdvanceSearchDelete(bool)));
        DELETE_OBJ(advanceCameraSearch);
    }

    disconnect (getAcqListTimer,
                SIGNAL(timeout()),
                this,
                SLOT(slotGetAcqListTimerTimeout()));
    delete getAcqListTimer;

    if (IS_VALID_OBJ(autoAddCameraButton))
    {
        disconnect (autoAddCameraButton,
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        disconnect (autoAddCameraButton,
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotButtonClick(int)));
        DELETE_OBJ(autoAddCameraButton);
    }

    failReasonList.clear();
    failIpAddresslist.clear();
    autoConfigCameraList.clear();
    autoConfigureStringList.clear();
    clearAllList();
}

void CameraSearch::createDefaultComponent()
{
    quint16 headerWidthArray[] = {35, 329, 68, 175, 175, 68, 55, 55};
    quint8 	headerTextLableYOffset[] = {14, 2, 14, 14, 2, 14, 14};
    quint8 	headerTextLableWidth[] = {120, 70, 150, 150, 51, 40, 81};
    quint8 	headerTextLableHeight[] = {21, 50, 25, 25, 50, 25, 50};

    testCameraIndex = INVALID_CAMERA_INDEX;
    currentPageNo = 0;
    isInfoPageLoaded = false;
    isCancelSend = false;
    currentFilterType = FILTER_NONE;

    advCamIpAddr1 = DFLT_IP_ADDR1;
    advCamIpAddr2 = DFLT_IP_ADDR2;
    advCamHttpPort = QString("%1").arg(DFLT_ADV_HTTP_PORT);

    for(quint8 index = 0; index < MAX_CAM_SRCH_ELEMENT; index++)
    {
        m_elementList[index] = NULL;
    }

    cameraSearchProcess = NULL;
    addCamera = NULL;
    testCam = NULL;
    advanceCameraSearch = NULL;
    camSearchFailReport = NULL;
    searchRunningInfo = NULL;
    m_currentElement = CAM_SRCH_SEARCH_BUTTON;
    autoConfigureCamera = NULL;
    autoAddButton = NULL;

    autoAddIconTimer = new QTimer();
    connect(autoAddIconTimer,
            SIGNAL(timeout()),
            this,
            SLOT(slotautoAddIconTimeOut()));
    autoAddIconTimer->setInterval(1000);

    QMap<quint8,QString> searchFilterListMap;
    searchFilterListMap.clear();
    for(quint8 index = 0; index < searchFilterList.length(); index++)
    {
        searchFilterListMap.insert (index,searchFilterList.at(index));
    }

    searchTypeDropDown = new DropDown((SCALE_WIDTH(PAGE_RIGHT_PANEL_WIDTH) - CAM_SERCH_BGTILE_WIDTH)/2,
                                      SCALE_HEIGHT(10),
                                      CAM_SERCH_BGTILE_WIDTH,
                                      BGTILE_HEIGHT,
                                      CAM_SRCH_SRCH_FILTER_DROPBOX,
                                      DROPDOWNBOX_SIZE_200,
                                      "Search Filter",
                                      searchFilterListMap,
                                      this,
                                      "",
                                      false,
                                      SCALE_WIDTH(20),
                                      COMMON_LAYER,
                                      false);
    m_elementList[CAM_SRCH_SRCH_FILTER_DROPBOX] = searchTypeDropDown;
    searchTypeDropDown->setIsEnabled(false);
    connect (searchTypeDropDown,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotFilterValueChanged(QString,quint32)));
    connect (searchTypeDropDown,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    autoAddButton = new CnfgButton(CNFGBUTTON_EXTRALARGE,
                                   (searchTypeDropDown->x() +
                                    searchTypeDropDown->width() - SCALE_WIDTH(120)),
                                   (searchTypeDropDown->y() +
                                    searchTypeDropDown->height()/2),
                                   cameraSearchStr[0],
                                   this,
                                   CAM_SRCH_AUTO_ADD,
                                   false);
    m_elementList[CAM_SRCH_AUTO_ADD] = autoAddButton;
    autoAddButton->setIsEnabled(false);
    connect (autoAddButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));
    connect (autoAddButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    topBgtile = new BgTile(searchTypeDropDown->BgTile::x(),
                           searchTypeDropDown->BgTile::height() + SCALE_HEIGHT(20),
                           CAM_SERCH_BGTILE_WIDTH,
                           (8*CAM_SERCH_FEILD_CELL_HEIGHT) - SCALE_HEIGHT(5),
                           TOP_LAYER,
                           this);

    bottomBgtile = new BgTile(topBgtile->x(),
                              topBgtile->y() + topBgtile->height(),
                              topBgtile->width(),
                              CAM_SERCH_FEILD_CELL_HEIGHT,
                              BOTTOM_LAYER,
                              this);

    fieldsHeading[0] = new TableCell(topBgtile->x() + SCALE_WIDTH(10),
                                     topBgtile->y() + SCALE_HEIGHT(10),
                                     (SCALE_WIDTH(headerWidthArray[0]) - 1),
                                     SCALE_HEIGHT(50),
                                     this,
                                     true);

    selectAllCam = new OptionSelectButton(fieldsHeading[0]->x() + SCALE_WIDTH(5),
                                          fieldsHeading[0]->y(),
                                          fieldsHeading[0]->width(),
                                          fieldsHeading[0]->height(),
                                          CHECK_BUTTON_INDEX,
                                          this,
                                          NO_LAYER,
                                          "","", -1,
                                          CAM_SRCH_SEL_ALL_CAM,
                                          false);
    m_elementList[CAM_SRCH_SEL_ALL_CAM] = selectAllCam;
    selectAllCam->setIsEnabled(false);
    connect (selectAllCam,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotOptionSelectionButton(OPTION_STATE_TYPE_e,int)));
    connect (selectAllCam,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    for(quint8 index = 1 ; index < MAX_RECORD_FEILDS ; index++)
    {
        fieldsHeading[index] = new TableCell(fieldsHeading[index -1]->x() +
                                             fieldsHeading[index -1]->width(),
                                             fieldsHeading[index -1]->y(),
                                             (SCALE_WIDTH(headerWidthArray[index]) - 1),
                                             SCALE_HEIGHT(50),
                                             this,
                                             true);

        elementHeading[index-1] = new ElementHeading(fieldsHeading[index -1]->x() +
                                                    fieldsHeading[index -1]->width() + SCALE_WIDTH(10),
                                                    fieldsHeading[index -1]->y() + SCALE_WIDTH(headerTextLableYOffset[index - 1]),
                                                    (SCALE_WIDTH(headerTextLableWidth[index - 1]) - 1),
                                                    SCALE_HEIGHT(headerTextLableHeight[index - 1]),
                                                    "",
                                                    NO_LAYER,
                                                    this,
                                                    false,
                                                    0,SCALE_WIDTH(12));

        QString fontColor = "#c8c8c8";
        QString fontWidth = "" + QString::number(SCALE_WIDTH(16)) +"px";
        QString styl = "ElidedLabel \
        { \
            color: %1; \
            font-size: %2; \
            font-family: %3; \
        }";
        elementHeading[index-1]->setStyleSheet(styl.arg(fontColor).arg(fontWidth).arg(NORMAL_FONT_FAMILY));
        elided[index-1] = new ElidedLabel(Multilang(cameraSearchStr[index].toUtf8().constData()), elementHeading[index-1]);
        elided[index-1]->resize((SCALE_WIDTH(headerWidthArray[index]) - 5),SCALE_HEIGHT(50));
        elided[index-1]->raise();
        elided[index-1]->show();
    }

    srNumber[0] = new TableCell(fieldsHeading[0]->x(),
                                fieldsHeading[0]->y() +
                                fieldsHeading[0]->height(),
                                fieldsHeading[0]->width(),
                                CAM_SERCH_FEILD_CELL_HEIGHT,
                                this);

    selectCam[0] = new OptionSelectButton(srNumber[0]->x() + SCALE_WIDTH(5),
                                          srNumber[0]->y(),
                                          srNumber[0]->width(),
                                          srNumber[0]->height(),
                                          CHECK_BUTTON_INDEX,
                                          this,
                                          NO_LAYER,
                                          "","", -1,
                                          CAM_SRCH_SEL_CAM,
                                          true);
    selectCam[0]->setVisible(false);
    selectCam[0]->setIsEnabled(false);
    m_elementList[CAM_SRCH_SEL_CAM] =  selectCam[0];
    connect (selectCam[0],
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotOptionSelectionButton(OPTION_STATE_TYPE_e,int)));
    connect (selectCam[0],
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    ipAddressCell[0] = new TableCell(fieldsHeading[1]->x(),
                                   fieldsHeading[1]->y() +
                                   fieldsHeading[1]->height(),
                                   fieldsHeading[1]->width(),
                                   CAM_SERCH_FEILD_CELL_HEIGHT,
                                   this);

    ipv4AddressStr[0] = new TextLabel(ipAddressCell[0]->x() + SCALE_WIDTH(10),
                                    ipAddressCell[0]->y() +
                                    (ipAddressCell[0]->height())/4,
                                    NORMAL_FONT_SIZE,
                                    "",
                                    this,
                                    NORMAL_FONT_COLOR,
                                    NORMAL_FONT_FAMILY,
                                    ALIGN_START_X_CENTRE_Y, 0, 0, SCALE_WIDTH(headerWidthArray[1]) - SCALE_WIDTH(12));

    ipv6AddressStr[0] = new TextLabel(ipAddressCell[0]->x() + SCALE_WIDTH(10),
                                      ipAddressCell[0]->y() +
                                      (ipAddressCell[0]->height() * 3)/4,
                                      NORMAL_FONT_SIZE,
                                      "",
                                      this,
                                      NORMAL_FONT_COLOR,
                                      NORMAL_FONT_FAMILY,
                                      ALIGN_START_X_CENTRE_Y, 0, 0, SCALE_WIDTH(headerWidthArray[1]) - SCALE_WIDTH(12));

    httpPorts[0] = new TableCell(fieldsHeading[2]->x(),
                                 fieldsHeading[2]->y() +
                                 fieldsHeading[2]->height(),
                                 fieldsHeading[2]->width(),
                                 CAM_SERCH_FEILD_CELL_HEIGHT,
                                 this);

    httpPortsStr[0] = new TextLabel(httpPorts[0]->x() + SCALE_WIDTH(10),
                                    httpPorts[0]->y() +
                                    (httpPorts[0]->height())/2,
                                    NORMAL_FONT_SIZE,
                                    "",
                                    this,
                                    NORMAL_FONT_COLOR,
                                    NORMAL_FONT_FAMILY,
                                    ALIGN_START_X_CENTRE_Y, 0, 0, SCALE_WIDTH(headerWidthArray[2]) - SCALE_WIDTH(12));

    brands[0] = new TableCell(fieldsHeading[3]->x(),
                              fieldsHeading[3]->y() +
                              fieldsHeading[3]->height(),
                              fieldsHeading[3]->width(),
                              CAM_SERCH_FEILD_CELL_HEIGHT,
                              this);

    brandsStr[0] = new TextLabel(brands[0]->x() + SCALE_WIDTH(10),
                                 brands[0]->y() +
                                 (brands[0]->height())/2,
                                 NORMAL_FONT_SIZE,
                                 "",
                                 this,
                                 NORMAL_FONT_COLOR,
                                 NORMAL_FONT_FAMILY,
                                 ALIGN_START_X_CENTRE_Y, 0, 0, SCALE_WIDTH(headerWidthArray[3]) - SCALE_WIDTH(12));

    model[0] = new TableCell(fieldsHeading[4]->x(),
                             fieldsHeading[4]->y() +
                             fieldsHeading[4]->height(),
                             fieldsHeading[4]->width(),
                             CAM_SERCH_FEILD_CELL_HEIGHT,
                             this);

    modelStr[0] = new TextLabel(model[0]->x() + SCALE_WIDTH(10),
                                model[0]->y() +
                                (model[0]->height())/2,
                                NORMAL_FONT_SIZE,
                                "",
                                this,
                                NORMAL_FONT_COLOR,
                                NORMAL_FONT_FAMILY,
                                ALIGN_START_X_CENTRE_Y, 0, 0, SCALE_WIDTH(headerWidthArray[4]) - SCALE_WIDTH(12));

    onvifPorts[0] = new TableCell(fieldsHeading[5]->x(),
                                  fieldsHeading[5]->y() +
                                  fieldsHeading[5]->height(),
                                  fieldsHeading[5]->width(),
                                  CAM_SERCH_FEILD_CELL_HEIGHT,
                                  this);

    onvifPortStr[0] = new TextLabel(onvifPorts[0]->x() + SCALE_WIDTH(10),
                                    onvifPorts[0]->y() +
                                    (onvifPorts[0]->height())/2,
                                    NORMAL_FONT_SIZE,
                                    "",
                                    this,
                                    NORMAL_FONT_COLOR,
                                    NORMAL_FONT_FAMILY,
                                    ALIGN_START_X_CENTRE_Y, 0, 0, SCALE_WIDTH(headerWidthArray[5]) - SCALE_WIDTH(12));

    addCameraCell[0] = new TableCell(fieldsHeading[6]->x(),
                                     fieldsHeading[6]->y() +
                                     fieldsHeading[6]->height(),
                                     fieldsHeading[6]->width(),
                                     CAM_SERCH_FEILD_CELL_HEIGHT,
                                     this);

    addCameraBtn[0] = new ControlButton(ADD_BUTTON_TABLE_INDEX,
                                        addCameraCell[0]->x() + SCALE_WIDTH(15),
                                        addCameraCell[0]->y(),
                                        addCameraCell[0]->width(),
                                        addCameraCell[0]->height(),
                                        this,
                                        NO_LAYER,
                                        -1,
                                        "",
                                        true,
                                        CAM_SRCH_ADD_CAM);
    m_elementList[CAM_SRCH_ADD_CAM] = addCameraBtn[0];
    connect (addCameraBtn[0],
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (addCameraBtn[0],
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotAddButtonClick(int)));
    addCameraBtn[0]->setVisible(false);
    addCameraBtn[0]->setIsEnabled(false);

    testCamera[0] = new TableCell(fieldsHeading[7]->x(),
                                  fieldsHeading[7]->y() +
                                  fieldsHeading[7]->height(),
                                  fieldsHeading[7]->width() -1,
                                  CAM_SERCH_FEILD_CELL_HEIGHT,
                                  this);

    testCameraBtn[0] = new ControlButton(TEST_CAMERAS_BUTTON_INDEX,
                                         testCamera[0]->x() + SCALE_WIDTH(13),
                                         testCamera[0]->y(),
                                         testCamera[0]->width(),
                                         testCamera[0]->height(),
                                         this,
                                         NO_LAYER,
                                         -1,
                                         "",
                                         true,
                                         (CAM_SRCH_TEST_CAM));
    m_elementList[(CAM_SRCH_TEST_CAM)] = testCameraBtn[0];
    connect (testCameraBtn[0],
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotTestButtonClick(int)));
    connect (testCameraBtn[0],
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    addCameraBtn[0]->setVisible(false);
    testCameraBtn[0]->setVisible(false);
    testCameraBtn[0]->setIsEnabled(false);

    for(quint8 index = 1 ; index < MAX_RECORD_DATA; index++)
    {
        srNumber[index] = new TableCell(srNumber[(index -1)]->x(),
                                        srNumber[(index -1)]->y() +
                                        srNumber[(index -1)]->height(),
                                        srNumber[(index -1)]->width() - 1,
                                        CAM_SERCH_FEILD_CELL_HEIGHT,
                                        this);

        selectCam[index] = new OptionSelectButton(srNumber[index]->x() + SCALE_WIDTH(5),
                                                  srNumber[index]->y(),
                                                  srNumber[index]->width(),
                                                  srNumber[index]->height(),
                                                  CHECK_BUTTON_INDEX,
                                                  this,
                                                  NO_LAYER,
                                                  "","", -1,
                                                  (CAM_SRCH_SEL_CAM + (index*MAX_NAVIGATE_CONTROL_IN_ROW)),
                                                  true);
        selectCam[index]->setVisible(false);
        selectCam[index]->setIsEnabled(false);
        m_elementList[(CAM_SRCH_SEL_CAM + (index*MAX_NAVIGATE_CONTROL_IN_ROW))] =  selectCam[index];
        connect (selectCam[index],
                 SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                 this,
                 SLOT(slotOptionSelectionButton(OPTION_STATE_TYPE_e,int)));
        connect (selectCam[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));

        ipAddressCell[index] = new TableCell(ipAddressCell[(index -1)]->x(),
        									 ipAddressCell[(index -1)]->y() +
                                           	 ipAddressCell[(index -1)]->height(),
                                           	 ipAddressCell[(index -1)]->width() - 1,
                                           	 CAM_SERCH_FEILD_CELL_HEIGHT,
                                           	 this);

        ipv4AddressStr[index] = new TextLabel(ipAddressCell[index]->x() + SCALE_WIDTH(10),
                                              ipAddressCell[index]->y() +
                                              (ipAddressCell[index]->height())/4,
                                              NORMAL_FONT_SIZE,
                                              "",
                                              this,
                                              NORMAL_FONT_COLOR,
                                              NORMAL_FONT_FAMILY,
                                              ALIGN_START_X_CENTRE_Y, 0, 0, SCALE_WIDTH(headerWidthArray[1]) - SCALE_WIDTH(12));

        ipv6AddressStr[index] = new TextLabel(ipAddressCell[index]->x() + SCALE_WIDTH(10),
                                             ipAddressCell[index]->y() +
                                             (ipAddressCell[index]->height() * 3)/4,
                                             NORMAL_FONT_SIZE,
                                             "",
                                             this,
                                             NORMAL_FONT_COLOR,
                                             NORMAL_FONT_FAMILY,
                                             ALIGN_START_X_CENTRE_Y, 0, 0, SCALE_WIDTH(headerWidthArray[1]) - SCALE_WIDTH(12));

        httpPorts[index] = new TableCell(httpPorts[(index -1)]->x(),
                                         httpPorts[(index -1)]->y() +
                                         httpPorts[(index -1)]->height(),
                                         httpPorts[(index -1)]->width() - 1,
                                         CAM_SERCH_FEILD_CELL_HEIGHT,
                                         this);

        httpPortsStr[index] = new TextLabel(httpPorts[index]->x() + SCALE_WIDTH(10),
                                            httpPorts[index]->y() +
                                            (httpPorts[index]->height())/2,
                                            NORMAL_FONT_SIZE,
                                            "",
                                            this,
                                            NORMAL_FONT_COLOR,
                                            NORMAL_FONT_FAMILY,
                                            ALIGN_START_X_CENTRE_Y, 0, 0, SCALE_WIDTH(headerWidthArray[2]) - SCALE_WIDTH(12));

        brands[index] = new TableCell(brands[(index -1)]->x(),
                                      brands[(index -1)]->y() +
                                      brands[(index -1)]->height(),
                                      brands[(index -1)]->width() - 1,
                                      CAM_SERCH_FEILD_CELL_HEIGHT,
                                      this);

        brandsStr[index] = new TextLabel(brands[index]->x() + SCALE_WIDTH(10),
                                         brands[index]->y() +
                                         (brands[index]->height())/2,
                                         NORMAL_FONT_SIZE,
                                         "",
                                         this,
                                         NORMAL_FONT_COLOR,
                                         NORMAL_FONT_FAMILY,
                                         ALIGN_START_X_CENTRE_Y, 0, 0, SCALE_WIDTH(headerWidthArray[3]) - SCALE_WIDTH(12));

        model[index] = new TableCell(model[(index -1)]->x(),
                                     model[(index -1)]->y() +
                                     model[(index -1)]->height(),
                                     model[(index -1)]->width() - 1,
                                     CAM_SERCH_FEILD_CELL_HEIGHT,
                                     this);

        modelStr[index] = new TextLabel(model[index]->x() + SCALE_WIDTH(10),
                                        model[index]->y() +
                                        (model[index]->height())/2,
                                        NORMAL_FONT_SIZE,
                                        "",
                                        this,
                                        NORMAL_FONT_COLOR,
                                        NORMAL_FONT_FAMILY,
                                        ALIGN_START_X_CENTRE_Y, 0, 0, SCALE_WIDTH(headerWidthArray[4]) - SCALE_WIDTH(12));

        onvifPorts[index] = new TableCell(onvifPorts[(index - 1)]->x(),
                                          onvifPorts[(index - 1)]->y() +
                                          onvifPorts[(index - 1)]->height(),
                                          onvifPorts[(index - 1)]->width() - 1,
                                          CAM_SERCH_FEILD_CELL_HEIGHT,
                                          this);

        onvifPortStr[index] = new TextLabel(onvifPorts[index]->x() + SCALE_WIDTH(10),
                                            onvifPorts[index]->y() +
                                            (onvifPorts[index]->height())/2,
                                            NORMAL_FONT_SIZE,
                                            "",
                                            this,
                                            NORMAL_FONT_COLOR,
                                            NORMAL_FONT_FAMILY,
                                            ALIGN_START_X_CENTRE_Y, 0, 0, SCALE_WIDTH(headerWidthArray[5]) - SCALE_WIDTH(12));

        addCameraCell[index] = new TableCell(addCameraCell[(index -1)]->x(),
                                             addCameraCell[(index -1)]->y() +
                                             addCameraCell[(index -1)]->height(),
                                             addCameraCell[(index -1)]->width() - 1,
                                             CAM_SERCH_FEILD_CELL_HEIGHT,
                                             this);

        addCameraBtn[index] = new ControlButton(ADD_BUTTON_TABLE_INDEX,
                                                addCameraCell[index]->x() + SCALE_WIDTH(15),
                                                addCameraCell[index]->y(),
                                                addCameraCell[index]->width(),
                                                addCameraCell[index]->height(),
                                                this,
                                                NO_LAYER,
                                                -1,
                                                "",
                                                true,
                                                (index*MAX_NAVIGATE_CONTROL_IN_ROW + CAM_SRCH_ADD_CAM));
        m_elementList[(index*MAX_NAVIGATE_CONTROL_IN_ROW + CAM_SRCH_ADD_CAM)] = addCameraBtn[index];
        connect (addCameraBtn[index],
                 SIGNAL(sigButtonClick(int)),
                 this,
                 SLOT(slotAddButtonClick(int)));
        connect (addCameraBtn[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
        addCameraBtn[index]->setVisible(false);
        addCameraBtn[index]->setIsEnabled(false);

        testCamera[index] = new TableCell(testCamera[(index -1)]->x(),
                                          testCamera[(index -1)]->y() +
                                          testCamera[(index -1)]->height(),
                                          testCamera[(index -1)]->width()- 1,
                                          CAM_SERCH_FEILD_CELL_HEIGHT,
                                          this);

        testCameraBtn[index] = new ControlButton(TEST_CAMERAS_BUTTON_INDEX,
                                                 testCamera[index]->x() + SCALE_WIDTH(13),
                                                 testCamera[index]->y(),
                                                 testCamera[index]->width(),
                                                 testCamera[index]->height(),
                                                 this,
                                                 NO_LAYER,
                                                 -1,
                                                 "",
                                                 true,
                                                 (index*MAX_NAVIGATE_CONTROL_IN_ROW + CAM_SRCH_TEST_CAM));
        m_elementList[(index*MAX_NAVIGATE_CONTROL_IN_ROW + CAM_SRCH_TEST_CAM)] = testCameraBtn[index];
        connect (testCameraBtn[index],
                 SIGNAL(sigButtonClick(int)),
                 this,
                 SLOT(slotTestButtonClick(int)));
        connect (testCameraBtn[index],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
        testCameraBtn[index]->setVisible(false);
        testCameraBtn[index]->setIsEnabled(false);
    }

    previousButton = new ControlButton(PREVIOUS_BUTTON_INDEX,
                                       topBgtile->x() + SCALE_WIDTH(15),
                                       bottomBgtile->y() + SCALE_HEIGHT(13),
                                       topBgtile->width(),
                                       BGTILE_HEIGHT,
                                       this,
                                       NO_LAYER,
                                       -1,
                                       cameraSearchStr[MAX_RECORD_FEILDS],
                                       false,
                                       CAM_SRCH_PREVIOUS_BUTTON,
                                       false);
    m_elementList[CAM_SRCH_PREVIOUS_BUTTON] = previousButton;
    connect(previousButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (previousButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));
    previousButton->setIsEnabled(false);

    searchButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                  topBgtile->x() + SCALE_WIDTH(400),
                                  previousButton->y() + SCALE_HEIGHT(18) ,
                                  cameraSearchStr[MAX_RECORD_FEILDS + 1],
                                  this,
                                  CAM_SRCH_SEARCH_BUTTON);
    m_elementList[CAM_SRCH_SEARCH_BUTTON] = searchButton;
    connect (searchButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (searchButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    stopButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                                  topBgtile->x() + SCALE_WIDTH(550),
                                  previousButton->y() + SCALE_HEIGHT(18),
                                  cameraSearchStr[MAX_RECORD_FEILDS + 2],
                                  this,
                                  CAM_SRCH_CANCEL_BUTTON,
                                  false);
    m_elementList[CAM_SRCH_CANCEL_BUTTON] = stopButton;
    connect(stopButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (stopButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    nextButton = new ControlButton(NEXT_BUTTON_INDEX,
                                   topBgtile->x() + topBgtile->width() - SCALE_WIDTH(90),
                                   bottomBgtile->y() + SCALE_HEIGHT(13),
                                   topBgtile->width(),
                                   BGTILE_HEIGHT,
                                   this,
                                   NO_LAYER,
                                   -1,
                                   cameraSearchStr[MAX_RECORD_FEILDS + 3],
                                   false,
                                   CAM_SRCH_NEXT_BUTTON);
    m_elementList[CAM_SRCH_NEXT_BUTTON] = nextButton;
    connect(nextButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (nextButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    advnaceSearchButton = new PageOpenButton((SCALE_WIDTH(PAGE_RIGHT_PANEL_WIDTH) - CAM_SERCH_BGTILE_WIDTH)/2,
                                             bottomBgtile->y() + bottomBgtile->height() + SCALE_HEIGHT(10),
                                             CAM_SERCH_BGTILE_WIDTH,
                                             BGTILE_HEIGHT,
                                             CAM_SRCH_ADVANCE_BUTTON,
                                             PAGEOPENBUTTON_ULTRALARGE,
                                             "Advance Camera Search",
                                             this,
                                             "","",
                                             false,
                                             SCALE_WIDTH(700));
    m_elementList[CAM_SRCH_ADVANCE_BUTTON] = advnaceSearchButton;
    connect(advnaceSearchButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect (advnaceSearchButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    failReportButton = new PageOpenButton(advnaceSearchButton->x() + SCALE_WIDTH(10),
                                          advnaceSearchButton->y(),
                                          advnaceSearchButton->width(),
                                          advnaceSearchButton->height(),
                                          CAM_SRCH_FAIL_REPORT_BUTTON,
                                          PAGEOPENBUTTON_ULTRALARGE,
                                          "Failure Report",
                                          this,
                                          "","",
                                          false,
                                          0,
                                          NO_LAYER,
                                          false);
    m_elementList[CAM_SRCH_FAIL_REPORT_BUTTON] = failReportButton;
    connect (failReportButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (failReportButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    autoAddCameraButton = new PageOpenButton((SCALE_WIDTH(PAGE_RIGHT_PANEL_WIDTH) - CAM_SERCH_BGTILE_WIDTH)/2,
                                             advnaceSearchButton->y()
                                             + failReportButton->height() + SCALE_HEIGHT(10),
                                             CAM_SERCH_BGTILE_WIDTH,
                                             BGTILE_HEIGHT,
                                             CAM_SRCH_AUTO_ADD_CAM_BUTTON,
                                             PAGEOPENBUTTON_ULTRALARGE_IMGOVERLAPED,
                                             "Auto Add Camera",
                                             this,
                                             "","",
                                             false,
                                             SCALE_WIDTH(10),
                                             COMMON_LAYER,
                                             true,
                                             ALIGN_START_X_CENTRE_Y);
    m_elementList[CAM_SRCH_AUTO_ADD_CAM_BUTTON] = autoAddCameraButton;
    connect (autoAddCameraButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));
    connect (autoAddCameraButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

#if defined(OEM_JCI)
    autoAddCameraButton->setVisible(false);
    autoAddCameraButton->setIsEnabled(false);
#endif
}

void CameraSearch::sendCommand(SET_COMMAND_e cmdType, qint32 totalfeilds)
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = cmdType;
    param->payload = payloadLib->createDevCmdPayload(totalfeilds);
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void CameraSearch::sendCamAutoSearchCmd()
{
    if(cameraSearchProcess == NULL)
    {
        cameraSearchProcess = new CameraSearchProcess(topBgtile->x(),
                                                      topBgtile->y(),
                                                      topBgtile->width(),
                                                      topBgtile->height() + SCALE_HEIGHT(10),
                                                      this);
    }

    isUpdateBeforeSearch = true;
    isCancelSend = false;
    updateControlsforSearch(true);
    failReportButton->setIsEnabled(false);
    currentPageNo = 0;
    isAdvanceCameraSearchRunning = false;

    // sending 0 which indicates that command is for search page
    payloadLib->setCnfgArrayAtIndex(0,0);
    sendCommand(AUTO_SEARCH,1);
}

void CameraSearch::handleInfoPageMessage(int)
{
    isInfoPageLoaded = false;
    if(currentFilterType != FILTER_ADDED)
    {
        autoAddIconTimer->start();
    }
}

void CameraSearch::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    isInfoPageLoaded = false;
    if(deviceName != currDevName)
    {
        return;
    }

    switch(param->msgType)
    {
        case MSG_SET_CMD:
        {
            switch(param->cmdType)
            {
                case AUTO_SEARCH:
                case ADV_CAM_SEARCH:
                {
                    DELETE_OBJ(cameraSearchProcess);
                    if(param->deviceStatus == CMD_SUCCESS)
                    {
                        isUpdateBeforeSearch = true;
                        updateControlsforSearch(true);
                        if((searchRunningInfo == NULL) && (isCancelSend == false))
                        {
                            searchRunningInfo = new TextLabel((SCALE_WIDTH(PAGE_RIGHT_PANEL_WIDTH) - CAM_SERCH_BGTILE_WIDTH/2 + SCALE_WIDTH(38)),
                                                              SCALE_HEIGHT(15),
                                                              SCALE_FONT(18),
                                                              Multilang("Searching...") +  QString(" 0 ") +  Multilang("camera(s) found"),
                                                              this,
                                                              HIGHLITED_FONT_COLOR,
                                                              NORMAL_FONT_FAMILY,
                                                              ALIGN_CENTRE_X_START_Y);
                        }

                        getAcqListTimer->start();
                        failReportButton->setIsEnabled((param->cmdType == ADV_CAM_SEARCH));
                    }
                    else
                    {
                        DELETE_OBJ(searchRunningInfo);
                        loadInfoPageMsg(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                        isUpdateBeforeSearch = false;
                        updateControlsforSearch(false);
                        failReportButton->setIsEnabled(false);
                        updateSelCamState();

                        fillFilterList();

                        if(param->cmdType == ADV_CAM_SEARCH)
                        {
                            isAdvanceCameraSearchRunning = false;
                        }
                        else
                        {
                            failReportButton->setIsEnabled(false);
                        }
                    }
                }
                break;

                case GET_ACQ_LIST:
                {
                    /* When AddCamera Page is open then List is not updated */
                    if(isBlockListFlag == false)
                    {
                        updateList(param);
                    }

                    searchTypeDropDown->setIsEnabled((cameraSearchList.isEmpty()) ? false : true);
                    if(isCancelSend == false)
                    {
                        getAcqListTimer->start();
                    }
                }
                break;

                case CNCL_AUTO_SEARCH:
                {
                    DELETE_OBJ(cameraSearchProcess);
                    DELETE_OBJ(searchRunningInfo);
                    isUpdateBeforeSearch = false;
                    updateControlsforSearch(false);
                    failReportButton->setIsEnabled(isAdvanceCameraSearchRunning);
                    isAdvanceCameraSearchRunning = false;
                    updateSelCamState();
                }
                break;

                case BRND_NAME:
                case MDL_NAME:
                case GET_USER_DETAIL:
                case GET_CAMERA_INFO:
                {
                    if(addCamera != NULL)
                    {
                        addCamera->processDeviceResponse(param, deviceName);
                    }

                    if(advanceCameraSearch != NULL)
                    {
                        if(param->cmdType == BRND_NAME)
                        {
                            isInfoPageLoaded = true;
                        }
                        advanceCameraSearch->processDeviceResponse(param, deviceName);
                    }
                }
                break;

                case ADD_CAMERAS:
                {
                    if(param->deviceStatus != CMD_SUCCESS)
                    {
                        loadInfoPageMsg(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                        isBlockListFlag = false;
                        break;
                    }

                    quint8 listIndex;

                    if (false == getCameraSearchListIndex(requestCameraParam, listIndex))
                    {
                        isBlockListFlag = false;
                        break;
                    }

                    payloadLib->parseDevCmdReply(true, param->payload);
                    quint16 cameraNum = payloadLib->getCnfgArrayAtIndex(0).toUInt();

                    CameraSearchParam cameraParam;
                    cameraParam = cameraSearchList.at(listIndex);
                    cameraParam.camName = requestCameraParam.camName;
                    cameraParam.camNum = cameraNum;
                    cameraParam.camStatus = MX_CAM_ADDED;
                    cameraSearchList.replace(listIndex, cameraParam);

                    fillFilterList();
                    showCameraSearchList();
                    updateSelCamState();

                    loadInfoPageMsg(ValidationMessage::getValidationMessage(CAM_SRCH_CAM_ADD_SUCCESS) + QString("%1").arg(cameraNum));
                    isBlockListFlag = false;
                }
                break;

                case OTHR_SUP:
                {
                    if(testCam != NULL)
                    {
                        testCam->processDeviceResponse(param,deviceName);
                    }
                }
                break;

                case TST_CAM:
                {
                    if(param->deviceStatus != CMD_SUCCESS)
                    {
                        testCameraIndex = INVALID_CAMERA_INDEX;
                        loadInfoPageMsg(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                        break;
                    }

                    if(testCam == NULL)
                    {
                        autoAddIconTimer->stop();
                        if(testCameraIndex != INVALID_CAMERA_INDEX)
                        {
                            testCam = new TestCamera(cameraSearchList.at(testCameraIndex).camNum,
                                                     cameraSearchList.at(testCameraIndex).camName,
                                                     payloadLib,
                                                     parentWidget());
                            connect (testCam,
                                     SIGNAL(sigCreateCMDRequest(SET_COMMAND_e,quint8)),
                                     this,
                                     SLOT(slotCreateCMDRequest(SET_COMMAND_e,quint8)));
                            connect (testCam,
                                     SIGNAL(sigDeleteObject(quint8)),
                                     this,
                                     SLOT(slotTestCamDelete(quint8)));
                        }
                    }
                }
                break;

                case GENERATE_FAILURE_REPORT:
                {
                    processBar->unloadProcessBar();
                    if(param->deviceStatus != CMD_SUCCESS)
                    {
                        loadInfoPageMsg(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                        break;
                    }

                    failIpAddresslist.clear();
                    failReasonList.clear();

                    payloadLib->parseDevCmdReply(true, param->payload);

                    for(quint8 index = 0; index < (payloadLib->getTotalCmdFields()/MAX_FAILURE_RESPOSE); index++)
                    {
                        failIpAddresslist.append(payloadLib->getCnfgArrayAtIndex(FAILURE_IP_ADDR + (index*MAX_FAILURE_RESPOSE)).toString());
                        failReasonList.append(failureStr[payloadLib->getCnfgArrayAtIndex(FAILURE_REASON + (index*MAX_FAILURE_RESPOSE)).toUInt()]);
                    }

                    if(camSearchFailReport == NULL)
                    {
                        /* PARASOFT: Memory Deallocated in slot FailReportDelete */
                        camSearchFailReport = new CamSearchFailReport(failIpAddresslist,
                                                                      failReasonList,
                                                                      parentWidget());
                        connect (camSearchFailReport,
                                 SIGNAL(sigObjectDelete()),
                                 this,
                                 SLOT(slotFailReportDelete()));
                    }
                }
                break;

                case MAX_ADD_CAM:
                {
                    if(m_autoAddCamera != NULL)
                    {
                        m_autoAddCamera->processDeviceResponse(param, deviceName);
                        break;
                    }

                    if(param->deviceStatus != CMD_SUCCESS)
                    {
                        loadInfoPageMsg(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                        break;
                    }

                    payloadLib->parseDevCmdReply(true, param->payload);
                    quint8 configCam = payloadLib->getCnfgArrayAtIndex(0).toUInt();
                    qint32 maxPossibleCam = (devTableInfo->ipCams - configCam);
                    if(maxPossibleCam <= 0)
                    {
                        loadInfoPageMsg(ValidationMessage::getDeviceResponceMessage(CMD_MAX_CAM_CONFIGED));
                        break;
                    }

                    if(autoConfigCameraList.length() > maxPossibleCam)
                    {
                        loadInfoPageMsg(QString("%1").arg(devTableInfo->ipCams - configCam) + " " + Multilang("camera(s) can be configured"));
                        break;
                    }

                    if(autoConfigureCamera == NULL)
                    {
                        /* PARASOFT: Memory Deallocated in slot ObjectDelete */
                        autoConfigureCamera = new AutoConfigureCamera(&autoConfigureStringList,
                                                                      currDevName, false,
                                                                      parentWidget());
                        connect (autoConfigureCamera,
                                 SIGNAL(sigObjectDelete()),
                                 this,
                                 SLOT(slotObjectDelete()));
                    }
                }
                break;

                case AUTO_CONFIGURE_CAMERA:
                {
                    processBar->unloadProcessBar();
                    autoConfigCameraList.clear();
                    if(param->deviceStatus == CMD_SUCCESS)
                    {
                        loadInfoPageMsg(ValidationMessage::getValidationMessage(CAM_SRCH_AUTO_CONFI));
                    }
                    else
                    {
                        loadInfoPageMsg(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                        isInfoPageLoaded = true;
                    }
                    updateSelCamState();
                }
                break;

                case GET_CAM_INITIATED_LIST:
                case ADD_CAM_INITIATED:
                case RJCT_CAM_INITIATED:
                {
                    if(m_autoAddCamera != NULL)
                    {
                        m_autoAddCamera->processDeviceResponse(param, deviceName);
                        isInfoPageLoaded = true;
                    }
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

        case MSG_GET_CFG:
        {
            if(param->deviceStatus == CMD_SUCCESS)
            {
                processBar->unloadProcessBar();

                payloadLib->parsePayload(param->msgType, param->payload);
                if(payloadLib->getcnfgTableIndex() == GENERAL_TABLE_INDEX)
                {
                    autoConfigureStringList.clear();
                    for(quint8 index = FIELD_AUTO_CONFIG_IP_RETAIN; index < CAM_SEARCH_AUTO_CNFG_FIELD; index++)
                    {
                        autoConfigureStringList.append((payloadLib->getCnfgArrayAtIndex(FIELD_AUTO_CONFIG + index + 1)).toString());
                    }
                    autoConfigureStringList.append((payloadLib->getCnfgArrayAtIndex(FIELD_AUTO_CONFIG_USERNAME)).toString());
                    autoConfigureStringList.append((payloadLib->getCnfgArrayAtIndex(FIELD_AUTO_CONFIG_PASSWORD)).toString());
                }
            }

            if(addCamera != NULL)
            {
                addCamera->processDeviceResponse(param, deviceName);
            }
        }
        break;

        case MSG_SET_CFG:
        {
            if(param->deviceStatus == CMD_SUCCESS)
            {
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
                slotObjectDelete();
                processBar->loadProcessBar(); // when save command successfully recived then go for AutoConfigure.
                sendAutoAddCmd();
            }
            else
            {
                infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                infoPage->raise();
            }
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }

    if((isInfoPageLoaded == false) && (addCamera == NULL))
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

void CameraSearch::showCameraSearchList()
{
    quint8              eleIndex;
    quint8              recordOnPage;
    CameraSearchParam   cameraParam;

    selectAllCam->setVisible(true);

    switch(currentFilterType)
    {
        case FILTER_ADDED:
            maxSearchListCount = cameraAddedList.length();
            break;

        case FILTER_NOT_ADDED:
            maxSearchListCount = cameraNotAddedList.length();
            break;

        default:
            maxSearchListCount = cameraSearchList.length();
            break;
    }

    maximumPages = (maxSearchListCount % MAX_RECORD_DATA == 0 ) ? (maxSearchListCount / MAX_RECORD_DATA) : ((maxSearchListCount / MAX_RECORD_DATA) + 1);
    if(maxSearchListCount < (MAX_RECORD_DATA*(currentPageNo + 1)))
    {
        recordOnPage = maxSearchListCount - ((MAX_RECORD_DATA*(currentPageNo)));
    }
    else
    {
        recordOnPage = MAX_RECORD_DATA;
    }

    if(((recordOnPage == 0) && (maxSearchListCount != 0)) || (maximumPages <= currentPageNo))
    {
        currentPageNo = 0;
        if(maxSearchListCount < (MAX_RECORD_DATA*(currentPageNo + 1)))
        {
            recordOnPage = maxSearchListCount - ((MAX_RECORD_DATA*(currentPageNo)));
        }
        else
        {
            recordOnPage = MAX_RECORD_DATA;
        }
    }

    if(recordOnPage != 0)
    {
        updateNavigationControlStatus();
    }
    else
    {
        previousButton->setIsEnabled(false);
        nextButton->setIsEnabled(false);
        currentPageNo = 0;
    }

    for(quint8 index = 0; index < recordOnPage; index++)
    {
        eleIndex = (index + (currentPageNo*MAX_RECORD_DATA));
        switch(currentFilterType)
        {
            case FILTER_ADDED:
                cameraParam = cameraAddedList.at(eleIndex);
                break;

            case FILTER_NOT_ADDED:
                cameraParam = cameraNotAddedList.at(eleIndex);
                break;

            default:
                cameraParam = cameraSearchList.at(eleIndex);
                break;
        }

        selectCam[index]->setVisible(true);

        if (cameraParam.ipv4Address.isEmpty())
        {
             ipv6AddressStr[index]->resetGeometry(ipAddressCell[index]->x() + SCALE_WIDTH(10) , ipAddressCell[index]->y() + ipAddressCell[index]->height()/2);
             ipv6AddressStr[index]->changeText(cameraParam.ipv6Address);
             ipv4AddressStr[index]->changeText("");
             ipv4AddressStr[index]->setVisible(false);
             ipv6AddressStr[index]->setVisible(true);
        }
        else if (cameraParam.ipv6Address.isEmpty())
        {
            ipv4AddressStr[index]->resetGeometry(ipAddressCell[index]->x() + SCALE_WIDTH(10), ipAddressCell[index]->y() + ipAddressCell[index]->height()/2);
            ipv6AddressStr[index]->changeText("");
            ipv4AddressStr[index]->changeText(cameraParam.ipv4Address);
            ipv6AddressStr[index]->setVisible(false);
            ipv4AddressStr[index]->setVisible(true);
        }
        else
        {
            ipv4AddressStr[index]->resetGeometry(ipAddressCell[index]->x() + SCALE_WIDTH(10), ipAddressCell[index]->y() + ipAddressCell[index]->height()/4);
            ipv6AddressStr[index]->resetGeometry(ipAddressCell[index]->x() + SCALE_WIDTH(10),ipAddressCell[index]->y() + (ipAddressCell[index]->height() * 3)/4);
            ipv4AddressStr[index]->changeText(cameraParam.ipv4Address);
            ipv6AddressStr[index]->changeText(cameraParam.ipv6Address);
            ipv4AddressStr[index]->setVisible(true);
            ipv6AddressStr[index]->setVisible(true);
        }
        ipv4AddressStr[index]->update();
        ipv6AddressStr[index]->update();

        httpPortsStr[index]->changeText(cameraParam.httpPort);
        httpPortsStr[index]->update();

        brandsStr[index]->changeText(cameraParam.brand);
        brandsStr[index]->update();

        modelStr[index]->changeText(cameraParam.model);
        modelStr[index]->update();

        onvifPortStr[index]->changeText((cameraParam.onvifSupportF == FALSE) ? DEFAULT_FIELD_VALUE_STR : cameraParam.onvifPort);
        onvifPortStr[index]->update();

        addCameraBtn[index]->setVisible(true);
        addCameraBtn[index]->setIsEnabled(true);

        addCameraBtn[index]->changeImageType((cameraParam.camStatus == MX_CAM_ADDED) ? ADDED_BUTTON_TABLE_INDEX : ADD_BUTTON_TABLE_INDEX);
        addCameraBtn[index]->update();

        testCameraBtn[index]->setIsEnabled(((cameraParam.camStatus == MX_CAM_ADDED) ? true : false));
        testCameraBtn[index]->setVisible(true);
    }

    if(recordOnPage != MAX_RECORD_DATA)
    {
        clearSerachDisplayList(recordOnPage);
    }
}

void CameraSearch::clearSerachDisplayList(quint8 recordOnPage)
{
    for(quint8 index = recordOnPage; index < MAX_RECORD_DATA; index++)
    {
        ipv4AddressStr[index]->changeText("");
        ipv6AddressStr[index]->changeText("");
        httpPortsStr[index]->changeText("");
        onvifPortStr[index]->changeText("");
        brandsStr[index]->changeText("");
        modelStr[index]->changeText("");
        addCameraBtn[index]->setVisible(false);
        addCameraBtn[index]->setIsEnabled(false);  // for navigation must disable
        testCameraBtn[index]->setVisible(false);
        testCameraBtn[index]->setIsEnabled(false);  // for navigation must disable
        selectCam[index]->setVisible(false);
        selectCam[index]->setIsEnabled(false);  // for navigation must disable
    }
}

void CameraSearch::clearAllList()
{
    cameraSearchList.clear();
    cameraAddedList.clear();
    cameraNotAddedList.clear();
}

void CameraSearch::updateList(DevCommParam *param)
{
    quint8              totalResult = 0;
    CameraSearchParam   cameraParam;

    clearAllList();
    if(param->deviceStatus == CMD_SUCCESS)
    {
        payloadLib->parseDevCmdReply(true,param->payload);
        totalResult = (payloadLib->getTotalCmdFields() / MAX_MX_CMD_CAM_SEARCH_FIELDS);
        for(quint8 index = 0; index < totalResult; index++)
        {
            cameraParam.camStatus = (MX_CAM_STATUS_e)payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_CAMERA_STATUS + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toUInt();
            cameraParam.camNum = payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_CAM_NUMBER + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toUInt();
            cameraParam.ipv4Address = payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_IPV4_ADDRESS + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString();
            cameraParam.ipv6Address = payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_IPV6_ADDRESS + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString();
            cameraParam.httpPort = payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_HTTP_PORT + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString();
            cameraParam.brand = payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_BRAND_NAME + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString();
            cameraParam.model = payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_MODEL_NAME + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString();
            cameraParam.onvifSupportF = payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_ONVIF_SUPPORT + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toBool();
            cameraParam.onvifPort = payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_ONVIF_PORT + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString();
            cameraParam.camName = payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_CAM_NAME + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString();

            if (cameraParam.camName == "")
            {
                cameraParam.camName = (cameraParam.brand != "") ?
                            cameraParam.brand : ((cameraParam.ipv4Address != "") ? cameraParam.ipv4Address : cameraParam.ipv6Address);
            }
            cameraSearchList.append(cameraParam);
        }
    }

    totalResult = maxSearchListCount = cameraSearchList.length();
    maximumPages = (maxSearchListCount % MAX_RECORD_DATA == 0 ) ? (maxSearchListCount / MAX_RECORD_DATA) : ((maxSearchListCount / MAX_RECORD_DATA) + 1);

    fillFilterList();
    showCameraSearchList();

    QString camFoundStr = Multilang("Searching...") + QString(" ") + QString("%1").arg(totalResult) + QString(" ") + Multilang("camera(s) found");
    if((searchRunningInfo == NULL) && (isCancelSend == false))
    {
        searchRunningInfo = new TextLabel((SCALE_WIDTH(PAGE_RIGHT_PANEL_WIDTH) - (CAM_SERCH_BGTILE_WIDTH)/2) + SCALE_WIDTH(38),
                                          SCALE_WIDTH(15),
                                          SCALE_FONT(18),
                                          camFoundStr,
                                          this,
                                          HIGHLITED_FONT_COLOR,
                                          NORMAL_FONT_FAMILY,
                                          ALIGN_CENTRE_X_START_Y);
    }
    else if(isCancelSend == true)
    {
        DELETE_OBJ(searchRunningInfo);
    }
    else if(searchRunningInfo != NULL)
    {
        searchRunningInfo->changeText(camFoundStr);
        searchRunningInfo->update();
    }

    fillAutoConfigList();
    updateSelCamState();
    autoAddButton->setIsEnabled(((cameraSearchList.isEmpty()) || (currentFilterType == FILTER_ADDED)) ? false : true);

    if((!cameraSearchList.isEmpty()) && (autoAddButton->getIsEnabled()))
    {
        if(!autoAddIconTimer->isActive())
        {
            autoAddIconTimer->start();
        }
    }
}

void CameraSearch::updateNavigationControlStatus()
{
    previousButton->setIsEnabled((currentPageNo != 0 ? true : false ));

    if(currentPageNo < (maximumPages - 1))
    {
        nextButton->setIsEnabled(true);
        if((m_currentElement != CAM_SRCH_PREVIOUS_BUTTON) || (currentPageNo == 0))
        {
            m_currentElement = CAM_SRCH_NEXT_BUTTON;
        }
    }
    else if(currentPageNo == (maximumPages - 1))
    {
        nextButton->setIsEnabled(false);
        m_currentElement = CAM_SRCH_PREVIOUS_BUTTON;
    }

    if(isInfoPageLoaded == false)
    {
        m_elementList[m_currentElement]->forceActiveFocus();
    }
}

void CameraSearch::cancelCamAutoSearchCmd()
{
    if(isOnbootAuoCamSearchRunning == false)
    {
        isCancelSend = true;
        sendCommand(CNCL_AUTO_SEARCH);
    }
}

void CameraSearch::fillFilterList()
{
    CameraSearchParam cameraParam;

    cameraAddedList.clear();
    cameraNotAddedList.clear();

    if (cameraSearchList.isEmpty())
    {
        return;
    }

    for(quint8 index = 0; index < maxSearchListCount; index++)
    {
        cameraParam = cameraSearchList.at(index);
        if (cameraParam.camStatus == MX_CAM_ADDED)
        {
            cameraAddedList.append(cameraParam);
        }
        else
        {
            cameraNotAddedList.append(cameraParam);
        }
    }
}

void CameraSearch::fillAutoConfigList()
{
    quint8              recordOnPage;
    CameraSearchParam   cameraParam;

    if(maxSearchListCount < (MAX_RECORD_DATA*(currentPageNo + 1)))
    {
        recordOnPage = maxSearchListCount - ((MAX_RECORD_DATA*(currentPageNo)));
    }
    else
    {
        recordOnPage = MAX_RECORD_DATA;
    }

    for(quint8 index = 0; index < recordOnPage; index++)
    {
        if ((false == selectCam[index]->getIsEnabled()) || (selectCam[index]->getCurrentState() == OFF_STATE))
        {
            continue;
        }

        quint8 listIndex = ((index) + (currentPageNo*(MAX_RECORD_DATA)));
        switch(currentFilterType)
        {
            case FILTER_ADDED:
                cameraParam = cameraAddedList.at(listIndex);
                break;

            case FILTER_NOT_ADDED:
                cameraParam = cameraNotAddedList.at(listIndex);
                break;

            default:
                cameraParam = cameraSearchList.at(listIndex);
                break;
        }

        if (false == getAutoConfigCameraListIndex(cameraParam, listIndex))
        {
            autoConfigCameraList.append(cameraParam);
        }
    }

    if(autoConfigCameraList.length() <= cameraSearchList.length())
    {
        return;
    }

    quint8 index = 0, listIndex = 0;
    while (autoConfigCameraList.length() > cameraSearchList.length())
    {
        if ((autoConfigCameraList.isEmpty() == true) || (listIndex >= autoConfigCameraList.length()))
        {
            break;
        }

        if (false == getCameraSearchListIndex(autoConfigCameraList.at(listIndex), index))
        {
            autoConfigCameraList.removeAt(listIndex);
        }
        else
        {
            listIndex++;
        }
    }
}

void CameraSearch::sendAutoAddCmd()
{
    quint8  totalAutoConfigCamera = autoConfigCameraList.length();
    qint32  autoConfigIpAddrFamily = QHostAddress(autoConfigureStringList.at(FIELD_AUTO_CONFIG_START_IP)).protocol();
    bool    retainIpAddrInAutoConfig = QVariant(autoConfigureStringList.at(FIELD_AUTO_CONFIG_IP_RETAIN)).toInt() ? true : false;

    for(quint8 index = 0; index < totalAutoConfigCamera; index++)
    {
        CameraSearchParam cameraParm = autoConfigCameraList.at(index);
        if (cameraParm.camName.size() > MAX_CAMERA_NAME_LENGTH)
        {
            cameraParm.camName.truncate(MAX_CAMERA_NAME_LENGTH);
        }

        payloadLib->setCnfgArrayAtIndex((MX_CMD_AUTO_ADD_FEILDS_CAM_STATUS + (MAX_MX_CMD_AUTO_ADD_FIELDS*index)), cameraParm.camStatus);
        payloadLib->setCnfgArrayAtIndex((MX_CMD_AUTO_ADD_FEILDS_CAM_NAME + (MAX_MX_CMD_AUTO_ADD_FIELDS*index)), cameraParm.camName);
        if ((retainIpAddrInAutoConfig == true) || (autoConfigIpAddrFamily == QAbstractSocket::IPv4Protocol))
        {
            payloadLib->setCnfgArrayAtIndex((MX_CMD_AUTO_ADD_FEILDS_IP_ADDRESS + (MAX_MX_CMD_AUTO_ADD_FIELDS*index)),
                                            ((cameraParm.ipv4Address.isEmpty() == false) ? cameraParm.ipv4Address : cameraParm.ipv6Address));
        }
        else
        {
            payloadLib->setCnfgArrayAtIndex((MX_CMD_AUTO_ADD_FEILDS_IP_ADDRESS + (MAX_MX_CMD_AUTO_ADD_FIELDS*index)),
                                            ((cameraParm.ipv6Address.isEmpty() == false) ? cameraParm.ipv6Address : cameraParm.ipv4Address));
        }
        payloadLib->setCnfgArrayAtIndex((MX_CMD_AUTO_ADD_FEILDS_HTTP_PORT + (MAX_MX_CMD_AUTO_ADD_FIELDS*index)), cameraParm.httpPort);
        payloadLib->setCnfgArrayAtIndex((MX_CMD_AUTO_ADD_FEILDS_BRAND_NAME + (MAX_MX_CMD_AUTO_ADD_FIELDS*index)), cameraParm.brand);
        payloadLib->setCnfgArrayAtIndex((MX_CMD_AUTO_ADD_FEILDS_MODEL_NAME + (MAX_MX_CMD_AUTO_ADD_FIELDS*index)), cameraParm.model);
        payloadLib->setCnfgArrayAtIndex((MX_CMD_AUTO_ADD_FEILDS_ONVIF_SUPPORT + (MAX_MX_CMD_AUTO_ADD_FIELDS*index)), cameraParm.onvifSupportF ? 1 : 0);
        payloadLib->setCnfgArrayAtIndex((AUTO_ADD_FEILDS_ONVIF_PORT + (MAX_MX_CMD_AUTO_ADD_FIELDS*index)), cameraParm.onvifPort);
    }

    autoConfigCameraList.clear();
    sendCommand(AUTO_CONFIGURE_CAMERA, (MAX_MX_CMD_AUTO_ADD_FIELDS*totalAutoConfigCamera));
}

void CameraSearch::updateSelCamState(bool isStateOn)
{
    bool                status = true;
    quint8              eleIndex = 0;
    quint8              recordOnPage = 0;
    quint8              totalOnState = 0;
    quint8              totalOffState = 0;
    MX_CAM_STATUS_e     camState;
    CameraSearchParam   cameraParam;

    if(cameraSearchList.isEmpty())
    {
        return;
    }

    if(maxSearchListCount < (MAX_RECORD_DATA*(currentPageNo + 1)))
    {
        recordOnPage = maxSearchListCount - ((MAX_RECORD_DATA*(currentPageNo)));
    }
    else
    {
        recordOnPage = MAX_RECORD_DATA;
    }

    for(quint8 index = 0; index < recordOnPage; index++)
    {
        eleIndex = ((index) + (currentPageNo*(MAX_RECORD_DATA)));
        switch(currentFilterType)
        {
            case FILTER_ADDED:
            {
                status = getCameraSearchListIndex(cameraAddedList.at(eleIndex), eleIndex);
            }
            break;

            case FILTER_NOT_ADDED:
            {
                status = getCameraSearchListIndex(cameraNotAddedList.at(eleIndex), eleIndex);
            }
            break;

            default:
            {
                status = true;
            }
            break;
        }

        if (status == false)
        {
            continue;
        }

        camState = cameraSearchList.at(eleIndex).camStatus;
        if ((camState == MX_CAM_ADDED) || ((cameraSearchList.at(eleIndex).onvifSupportF == false) && (camState == MX_CAM_UNIDENTIFY)))
        {
            selectCam[index]->changeState(OFF_STATE);
            selectCam[index]->setIsEnabled(false);
            totalOffState++;
            continue;
        }

        selectCam[index]->setIsEnabled(true);
        if (isStateOn)
        {
            selectCam[index]->changeState(ON_STATE);
            totalOnState++;
            continue;
        }

        selectCam[index]->changeState(OFF_STATE);
        if (autoConfigCameraList.isEmpty())
        {
           continue;
        }

        cameraParam.ipv4Address = ipv4AddressStr[index]->getText();
        cameraParam.ipv6Address = ipv6AddressStr[index]->getText();
        cameraParam.onvifSupportF = cameraSearchList.at(eleIndex).onvifSupportF;
        cameraParam.onvifPort = cameraSearchList.at(eleIndex).onvifPort;
        cameraParam.httpPort = httpPortsStr[index]->getText();

        if (true == getAutoConfigCameraListIndex(cameraParam, eleIndex))
        {
            selectCam[index]->changeState(ON_STATE);
            totalOnState++;
        }
    }

    if(((recordOnPage - totalOnState) == totalOffState) && (totalOffState != recordOnPage))
    {
        selectAllCam->changeState(ON_STATE);
    }
    else
    {
        selectAllCam->changeState(OFF_STATE);
    }

    selectAllCam->setIsEnabled((currentFilterType == FILTER_ADDED) ? false : (totalOffState == recordOnPage) ? false : true);
}

void CameraSearch::updateControlsforSearch(bool enableContrl)
{
    testCameraIndex = INVALID_CAMERA_INDEX;

    searchButton->setIsEnabled(!enableContrl);
    advnaceSearchButton->setIsEnabled(!enableContrl);
    stopButton->setIsEnabled(enableContrl);

    searchTypeDropDown->setIsEnabled((isUpdateBeforeSearch) ? false : (cameraSearchList.isEmpty()) ? false : true);
    autoAddButton->setIsEnabled((isUpdateBeforeSearch) ? false : (cameraSearchList.isEmpty()) ? false : true);

    if(isUpdateBeforeSearch)
    {
        clearAllList();
        currentFilterType = FILTER_NONE;
        showCameraSearchList();

        m_currentElement = CAM_SRCH_CANCEL_BUTTON;
        searchTypeDropDown->setIndexofCurrElement(0);
        selectAllCam->setIsEnabled(false);

        autoAddIconTimer->stop();
        autoConfigCameraList.clear();
    }
    else
    {
        if((!cameraSearchList.isEmpty()) && (autoAddButton->getIsEnabled()))
        {
            if(!autoAddIconTimer->isActive())
            {
                autoAddIconTimer->start();
            }
        }

        m_currentElement = CAM_SRCH_SEARCH_BUTTON;
    }
    m_elementList[m_currentElement]->forceActiveFocus();
}

void CameraSearch::loadInfoPageMsg(QString infoMsg)
{
    autoAddIconTimer->stop();
    isInfoPageLoaded = true;
    infoPage->loadInfoPage (infoMsg);
}

void CameraSearch::slotButtonClick(int indexInPage)
{
    switch(indexInPage)
    {
        case CAM_SRCH_AUTO_ADD:
        {
            fillAutoConfigList();
            if(autoConfigCameraList.isEmpty())
            {
                loadInfoPageMsg(ValidationMessage::getValidationMessage(CAM_SRCH_SELECT_CAM_TO_AUTO_CONFI));
            }
            else
            {
                sendCommand(MAX_ADD_CAM);
            }
        }
        break;

        case CAM_SRCH_PREVIOUS_BUTTON:
        {
            fillAutoConfigList();
            if(currentPageNo > 0)
            {
                currentPageNo --;
            }
            showCameraSearchList();
            updateSelCamState();
        }
        break;

        case CAM_SRCH_SEARCH_BUTTON:
        {
            if(isOnbootAuoCamSearchRunning == true)
            {
                loadInfoPageMsg(ValidationMessage::getDeviceResponceMessage(CMD_REQUEST_IN_PROGRESS));
            }
            else
            {
                sendCamAutoSearchCmd();
            }
        }
        break;

        case CAM_SRCH_CANCEL_BUTTON:
        {
            cancelCamAutoSearchCmd();
        }
        break;

        case CAM_SRCH_NEXT_BUTTON:
        {
            fillAutoConfigList();
            if (currentPageNo != (maximumPages - 1))
            {
                currentPageNo ++;
            }
            showCameraSearchList();
            updateSelCamState();
        }
        break;

        case CAM_SRCH_ADVANCE_BUTTON:
        {
            if(isOnbootAuoCamSearchRunning == true)
            {
                loadInfoPageMsg(ValidationMessage::getDeviceResponceMessage(CMD_REQUEST_IN_PROGRESS));
            }
            else if(advanceCameraSearch == NULL)
            {
                autoAddIconTimer->stop();
                advanceCameraSearch = new AdvanceCameraSearch(advCamIpAddr1,
                                                              advCamIpAddr2,
                                                              advCamHttpPort,
                                                              payloadLib,
                                                              parentWidget());
                connect (advanceCameraSearch,
                         SIGNAL(sigCreateCMDRequest(SET_COMMAND_e,quint8)),
                         this,
                         SLOT(slotCreateCMDRequest (SET_COMMAND_e,quint8)));
                connect (advanceCameraSearch,
                         SIGNAL(sigObjectDelete(bool)),
                         this,
                         SLOT(slotAdvanceSearchDelete(bool)));
                connect (advanceCameraSearch,
                         SIGNAL(sigAdvanceSearchRequest(QString,QString,QString)),
                         this,
                         SLOT(slotAdvanceSearchRange(QString,QString,QString)));
            }
        }
        break;

        case CAM_SRCH_FAIL_REPORT_BUTTON:
        {
            processBar->loadProcessBar();
            sendCommand(GENERATE_FAILURE_REPORT);
        }
        break;

        case CAM_SRCH_AUTO_ADD_CAM_BUTTON:
        {
            if(m_autoAddCamera == NULL)
            {
                /* PARASOFT: Memory Deallocated in slot AdvanceSearchDelete */
                m_autoAddCamera = new AutoAddCameraList(currDevName, parentWidget(),payloadLib);
                connect (m_autoAddCamera,
                         SIGNAL(sigObjectDelete(bool)),
                         this,
                         SLOT(slotAdvanceSearchDelete(bool)));
            }
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void CameraSearch::slotCreateCMDRequest(SET_COMMAND_e cmdType,quint8 totalFeilds)
{
    sendCommand(cmdType,totalFeilds);
}

void CameraSearch::slotAddCameraDelete(quint8 listIndex,
                                       bool saveCameraFlag,
                                       QString ipAddressString,
                                       QString httpPortStr,
                                       QString onvifPortString,
                                       bool onvifSupportF,
                                       QString brandListStr,
                                       QString modelListStr,
                                       QString camName,
                                       QString userName,
                                       QString tPassword,
                                       quint8 selIndex,
                                       quint8 currentIndex)
{
    Q_UNUSED(currentIndex);
    if(addCamera != NULL)
    {
        disconnect (addCamera,
                    SIGNAL(sigDeleteObject(quint8,bool,QString,QString,
                                           QString,bool,QString,QString,
                                           QString,QString,QString,quint8,quint8)),
                    this,
                    SLOT(slotAddCameraDelete(quint8,bool,QString,QString,
                                             QString,bool,QString,QString,
                                             QString,QString,QString,quint8,quint8)));
        DELETE_OBJ(addCamera);
        isBlockListFlag = false;
    }

    if(currentFilterType == FILTER_ADDED)
    {
        if(autoAddIconTimer->isActive())
        {
            autoAddIconTimer->stop();
        }
    }
    else
    {
        if(!autoAddIconTimer->isActive())
        {
            autoAddIconTimer->start();
        }
    }

    if(saveCameraFlag)
    {
        this->username = userName;
        this->password = tPassword;

        requestCameraParam = cameraSearchList.at(listIndex);
        requestCameraParam.camName = camName;
        requestCameraParam.onvifSupportF = onvifSupportF;

        payloadLib->setCnfgArrayAtIndex(0, selIndex);
        payloadLib->setCnfgArrayAtIndex(1, camName);
        payloadLib->setCnfgArrayAtIndex(2, ipAddressString);
        payloadLib->setCnfgArrayAtIndex(3, httpPortStr);

        if(onvifSupportF)
        {
            payloadLib->setCnfgArrayAtIndex(4, "");
            payloadLib->setCnfgArrayAtIndex(5, "");
        }
        else
        {
            payloadLib->setCnfgArrayAtIndex(4, brandListStr);
            payloadLib->setCnfgArrayAtIndex(5, modelListStr);
            requestCameraParam.brand = brandListStr;
            requestCameraParam.model = modelListStr;
        }

        payloadLib->setCnfgArrayAtIndex(6, userName);
        payloadLib->setCnfgArrayAtIndex(7, tPassword);
        payloadLib->setCnfgArrayAtIndex(8, ((onvifSupportF == true) ? 1 : 0));
        payloadLib->setCnfgArrayAtIndex(9, onvifPortString);


        cameraSearchList.replace(listIndex, requestCameraParam);
        fillFilterList();

        sendCommand(ADD_CAMERAS, 10);
    }
}

void CameraSearch::slotTestCamDelete(quint8)
{
    if(testCam != NULL)
    {
        disconnect (testCam,
                    SIGNAL(sigCreateCMDRequest(SET_COMMAND_e,quint8)),
                    this,
                    SLOT(slotCreateCMDRequest(SET_COMMAND_e,quint8)));
        disconnect (testCam,
                    SIGNAL(sigDeleteObject(quint8)),
                    this,
                    SLOT(slotTestCamDelete(quint8)));
        DELETE_OBJ(testCam);
    }

    if(currentFilterType == FILTER_ADDED)
    {
        if(autoAddIconTimer->isActive())
        {
            autoAddIconTimer->stop();
        }
    }
    else
    {
        if(!autoAddIconTimer->isActive())
        {
            autoAddIconTimer->start();
        }
    }
}

void CameraSearch::slotAdvanceSearchDelete(bool isCmdSend)
{
    if(isCmdSend)
    {
        if(cameraSearchProcess == NULL)
        {
            cameraSearchProcess = new CameraSearchProcess(topBgtile->x(),
                                                          topBgtile->y(),
                                                          topBgtile->width(),
                                                          topBgtile->height() + SCALE_HEIGHT(10),
                                                          this);
        }

        isAdvanceCameraSearchRunning = true;
        isUpdateBeforeSearch = true;
        isCancelSend = false;
        currentPageNo = 0;
        updateControlsforSearch(true);
    }

    if(advanceCameraSearch != NULL)
    {
        disconnect (advanceCameraSearch,
                    SIGNAL(sigCreateCMDRequest(SET_COMMAND_e,quint8)),
                    this,
                    SLOT(slotCreateCMDRequest (SET_COMMAND_e,quint8)));
        disconnect (advanceCameraSearch,
                    SIGNAL(sigObjectDelete(bool)),
                    this,
                    SLOT(slotAdvanceSearchDelete(bool)));
        DELETE_OBJ(advanceCameraSearch);
    }

    if(IS_VALID_OBJ(m_autoAddCamera))
    {
        disconnect (m_autoAddCamera,
                 SIGNAL(sigObjectDelete(bool)),
                 this,
                 SLOT(slotAdvanceSearchDelete(bool)));
        DELETE_OBJ(m_autoAddCamera);
    }

    if(isCmdSend == false)
    {
        if(currentFilterType == FILTER_ADDED)
        {
            if(autoAddIconTimer->isActive())
            {
                autoAddIconTimer->stop();
            }
        }
        else
        {
            if(!autoAddIconTimer->isActive())
            {
                autoAddIconTimer->start();
            }
        }
    }
}

void CameraSearch::slotFailReportDelete()
{
    if(camSearchFailReport != NULL)
    {
        disconnect (camSearchFailReport,
                    SIGNAL(sigObjectDelete()),
                    this,
                    SLOT(slotFailReportDelete()));
        DELETE_OBJ(camSearchFailReport);
    }

    if(currentFilterType == FILTER_ADDED)
    {
        if(autoAddIconTimer->isActive())
        {
            autoAddIconTimer->stop();
        }
    }
    else
    {
        if(!autoAddIconTimer->isActive())
        {
            autoAddIconTimer->start();
        }
    }
}

void CameraSearch::slotFilterValueChanged(QString str, quint32)
{
    fillAutoConfigList();

    currentFilterType = (SEARCH_FILTER_TYPE_e) searchFilterList.indexOf(str);
    autoAddButton->setIsEnabled((currentFilterType == FILTER_ADDED) ? false : true);
    currentPageNo = 0;

    showCameraSearchList();
    updateSelCamState();

    if(currentFilterType == FILTER_ADDED)
    {
        if(autoAddIconTimer->isActive())
        {
            autoAddIconTimer->stop();
        }
    }
    else
    {
        if(!autoAddIconTimer->isActive())
        {
            autoAddIconTimer->start();
        }
    }
}

void CameraSearch::slotOptionSelectionButton(OPTION_STATE_TYPE_e state, int indexInPage)
{
    bool                status = true;
    MX_CAM_STATUS_e     camState = MX_CAM_UNIDENTIFY;
    quint8              recordOnPage = 0;
    quint8              eleIndex = 0;
    quint8              totalOnState = 0;
    quint8              totalOffState = 0;
    CameraSearchParam   cameraParam;

    if(maxSearchListCount < (MAX_RECORD_DATA*(currentPageNo + 1)))
    {
        recordOnPage = maxSearchListCount - ((MAX_RECORD_DATA*(currentPageNo)));
    }
    else
    {
        recordOnPage = MAX_RECORD_DATA;
    }

    if(indexInPage == CAM_SRCH_SEL_ALL_CAM)
    {
        if(state == ON_STATE)
        {
            updateSelCamState(true);
        }
        else
        {
            for(quint8 index = 0; index < recordOnPage; index++)
            {
                selectCam[index]->changeState(OFF_STATE);
                eleIndex = ((index) + (currentPageNo*(MAX_RECORD_DATA)));
                switch(currentFilterType)
                {
                    case FILTER_ADDED:
                        cameraParam = cameraAddedList.at(eleIndex);
                        break;

                    case FILTER_NOT_ADDED:
                        cameraParam = cameraNotAddedList.at(eleIndex);
                        break;

                    default:
                        cameraParam = cameraSearchList.at(eleIndex);
                        break;
                }

                if (true == getAutoConfigCameraListIndex(cameraParam, eleIndex))
                {
                    autoConfigCameraList.removeAt(eleIndex);
                }
            }
        }
    }
    else
    {
        for(quint8 index = 0; index < recordOnPage; index++)
        {
            eleIndex = ((index) + (currentPageNo*(MAX_RECORD_DATA)));
            switch(currentFilterType)
            {
                case FILTER_ADDED:
                {
                    status = getCameraSearchListIndex(cameraAddedList.at(eleIndex), eleIndex);
                }
                break;

                case FILTER_NOT_ADDED:
                {
                    status = getCameraSearchListIndex(cameraNotAddedList.at(eleIndex), eleIndex);
                }
                break;

                default:
                {
                    status = true;
                }
                break;
            }

            if (status == false)
            {
                continue;
            }

            camState = cameraSearchList.at(eleIndex).camStatus;
            if(camState == MX_CAM_ADDED)
            {
                totalOffState++;
            }
            else if((cameraSearchList.at(eleIndex).onvifSupportF == false) && (camState == MX_CAM_UNIDENTIFY))
            {
                totalOffState++;
            }
            else
            {
                if(selectCam[index]->getCurrentState() == ON_STATE)
                {
                    totalOnState++;
                }
            }
        }

        if(((recordOnPage - totalOnState) == totalOffState) && (totalOffState != recordOnPage))
        {
            selectAllCam->changeState(ON_STATE);
        }
        else
        {
            selectAllCam->changeState(OFF_STATE);
        }

        if (state == OFF_STATE)
        {
            quint8 index = ((indexInPage/MAX_NAVIGATE_CONTROL_IN_ROW) - 1);
            quint8 tIndex = ((index) + (currentPageNo*(MAX_RECORD_DATA)));

            cameraParam.ipv4Address = ipv4AddressStr[index]->getText();
            cameraParam.ipv6Address = ipv6AddressStr[index]->getText();
            cameraParam.onvifSupportF = cameraSearchList.at(tIndex).onvifSupportF;
            cameraParam.onvifPort = cameraSearchList.at(tIndex).onvifPort;
            cameraParam.httpPort = httpPortsStr[index]->getText();

            if (true == getAutoConfigCameraListIndex(cameraParam, eleIndex))
            {
                autoConfigCameraList.removeAt(eleIndex);
            }
        }
    }
    fillAutoConfigList();
}

void CameraSearch::slotAddButtonClick(int indexInPage)
{
    bool status = true;
    quint8 eleIndex = (((indexInPage/MAX_NAVIGATE_CONTROL_IN_ROW) - 1) + (currentPageNo*(MAX_RECORD_DATA)));

    autoAddIconTimer->stop();
    switch(currentFilterType)
    {
        case FILTER_ADDED:
        {
            status = getCameraSearchListIndex(cameraAddedList.at(eleIndex), eleIndex);
        }
        break;

        case FILTER_NOT_ADDED:
        {
            status = getCameraSearchListIndex(cameraNotAddedList.at(eleIndex), eleIndex);
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }

    if (status == false)
    {
        return;
    }

    CameraSearchParam cameraParam = cameraSearchList.at(eleIndex);
    requestCameraParam.camName = cameraParam.camName;
    quint8 mapIndex = 0;

    if(addCamera == NULL)
    {
        isBlockListFlag = true;
        QMap<quint8, QString> ipAddrListMap;

        if (cameraParam.ipv4Address.isEmpty() != TRUE)
        {
            ipAddrListMap.insert(mapIndex++, cameraParam.ipv4Address);
        }

        if (cameraParam.ipv6Address.isEmpty() != TRUE)
        {
            ipAddrListMap.insert(mapIndex++, cameraParam.ipv6Address);
        }

        addCamera = new AddCamera(currDevName,
                                  ipAddrListMap,
                                  cameraParam.httpPort,
                                  cameraParam.onvifPort,
                                  cameraParam.onvifSupportF,
                                  cameraParam.brand,
                                  cameraParam.model,
                                  cameraParam.camName,
                                  username,
                                  password,
                                  parentWidget(),
                                  eleIndex,
                                  cameraParam.camStatus,
                                  cameraParam.camNum,
                                  devTableInfo);
        connect (addCamera,
                 SIGNAL(sigDeleteObject(quint8,bool,QString,QString,
                                        QString,bool,QString,QString,
                                        QString,QString,QString,quint8,quint8)),
                 this,
                 SLOT(slotAddCameraDelete(quint8,bool,QString,QString,
                                          QString,bool,QString,QString,
                                          QString,QString,QString,quint8,quint8)));
    }
}

void CameraSearch::slotTestButtonClick(int indexInPage)
{
    bool status = true;
    quint8 eleIndex = (((indexInPage/MAX_NAVIGATE_CONTROL_IN_ROW) - 1) + (currentPageNo*(MAX_RECORD_DATA)));

    autoAddIconTimer->stop();
    switch(currentFilterType)
    {
        case FILTER_ADDED:
        {
            status = getCameraSearchListIndex(cameraAddedList.at(eleIndex), eleIndex);
        }
        break;

        case FILTER_NOT_ADDED:
        {
            status = getCameraSearchListIndex(cameraNotAddedList.at(eleIndex), eleIndex);
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }

    if (status == false)
    {
        return;
    }

    testCameraIndex = eleIndex;
    payloadLib->setCnfgArrayAtIndex(0, cameraSearchList.at(eleIndex).camNum);
    sendCommand(TST_CAM, 1);
}

void CameraSearch::slotautoAddIconTimeOut()
{
    if((isInfoPageLoaded == false) && (autoAddButton->getIsEnabled()))
    {
        if(autoAddButton->getCurrentImageType() == IMAGE_TYPE_MOUSE_HOVER)
        {
            autoAddButton->changeImage(IMAGE_TYPE_NORMAL);
        }
        else
        {
            autoAddButton->changeImage(IMAGE_TYPE_MOUSE_HOVER,true);
        }
    }
}

void CameraSearch::slotAdvanceSearchRange(QString ipAddr1, QString ipAddr2, QString port)
{
    advCamIpAddr1 = ipAddr1;
    advCamIpAddr2 = ipAddr2;
    advCamHttpPort = port;
}

void CameraSearch::slotGetAcqListTimerTimeout()
{
    if(isCancelSend == false)
    {
        sendCommand(GET_ACQ_LIST);
    }
}

void CameraSearch::getConfig()
{
    //create the payload for Get Cnfg
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                             GENERAL_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_TO_INDEX,
                                                             CNFG_FRM_FIELD,
                                                             MAX_FIELD_NO,
                                                             0);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_GET_CFG;
    param->payload = payloadString;
    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void CameraSearch::slotObjectDelete()
{
    if(IS_VALID_OBJ(autoConfigureCamera))
    {
        disconnect (autoConfigureCamera,
                    SIGNAL(sigObjectDelete()),
                    this,
                    SLOT(slotObjectDelete()));
        DELETE_OBJ(autoConfigureCamera);
    }
}

bool CameraSearch::getCameraSearchListIndex(const CameraSearchParam &cameraParam, quint8 &listIndex)
{
    for(quint8 index = 0; index < cameraSearchList.length(); index++)
    {
        if (cameraSearchList.at(index) == cameraParam)
        {
            listIndex = index;
            return true;
        }
    }

    return false;
}

bool CameraSearch::getAutoConfigCameraListIndex(const CameraSearchParam &cameraParam, quint8 &listIndex)
{
    for(quint8 index = 0; index < autoConfigCameraList.length(); index++)
    {
        if (autoConfigCameraList.at(index) == cameraParam)
        {
            listIndex = index;
            return true;
        }
    }

    return false;
}
