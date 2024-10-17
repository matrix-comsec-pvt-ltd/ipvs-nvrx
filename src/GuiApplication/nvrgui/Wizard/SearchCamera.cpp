#include "SearchCamera.h"
#include "MxCommandFields.h"
#include "ValidationMessage.h"
#include "Controls/MessageBanner.h"
#include "ConfigPages/BasicSettings/AutoConfigureCamera.h"

#define MAX_NAVIGATE_CONTROL_IN_ROW         3
#define CAM_ADD_NO                          0
#define MAX_NAVIGATE_CONTROL_IN_ROW         3
#define CAMERA_SETTING_FROM_FIELD           1
#define CAMERA_SETTING_TO_FIELD_IP_TABLE    16
#define CNFG_TO_INDEX                       1
#define CAM_SEARCH_AUTO_CNFG_FIELD          20
#define CAM_SERCH_BGTILE_WIDTH              SCALE_WIDTH(970)
#define CAM_SEARCH_HEADER_CELL_HEIGHT       SCALE_HEIGHT(35)
#define CAM_SEARCH_TABLE_CELL_HEIGHT        SCALE_HEIGHT(60)
#define DEFAULT_FIELD_VALUE_STR             "-"

typedef enum
{
    CAM_SRCH_AUTO_ADD,
    CAM_SRCH_SEL_ALL_CAM = 2,
    CAM_SRCH_SEL_CAM = 3,
    CAM_SRCH_ADD_CAM,
    CAM_SRCH_TEST_CAM,
    CAM_SRCH_PREVIOUS_BUTTON = (CAM_SRCH_SEL_ALL_CAM + 3*MAX_CAM_SEARCH_RECORD_DATA),
    CAM_SRCH_SEARCH_BUTTON,
    CAM_SRCH_CANCEL_BUTTON,
    CAM_SRCH_NEXT_BUTTON,
    MAX_CAM_SRCH_ELEMENT
}CAM_SRCH_ELEMENT_e;

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

SearchCamera::SearchCamera(QString devName, QString subHeadStr, QWidget *parent, WIZARD_PAGE_INDEXES_e pageId) :WizardCommon(parent, pageId)
{
    loadInfoPageFlag = false;
    isBlockListFlag = false;
    deviceReponseForFailReport = CMD_SUCCESS;
    retainIpAddrInAutoConfig = true;
    autoConfigIpAddrFamily = 0;
    for(quint8 index = 0; index < MAX_CAM_SEARCH_RECORD_DATA; index++)
    {
        srNumberStr[index] = NULL;
    }
    currDevName = devName;
    applController = ApplController::getInstance ();
    payloadLib = new PayloadLib();

    WizardCommon:: LoadProcessBar();
    createDefaultElements(subHeadStr);
    getAcqListTimer = new QTimer(this);
    connect (getAcqListTimer,
             SIGNAL(timeout()),
             this,
             SLOT(slotGetAcqListTimerTimeout()));
    getAcqListTimer->setInterval (3000);
    getAcqListTimer->setSingleShot (true);

    applController->GetDeviceInfo (currDevName, devTableInfo);
    getConfig();
    sendCamAutoSearchCmd();
    this->show();
}

void SearchCamera::createDefaultElements(QString subHeadStr)
{
    quint16 headerWidthArray[] = {25, 330, 90, 195, 190, 105, 55, 55};

    testCameraIndex = INVALID_CAMERA_INDEX;
    currentPageNo = 0;
    isInfoPageLoaded = false;
    isCancelSend = false;
    cameraSearchProcess = NULL;
    addCamera = NULL;
    testCam = NULL;
    searchRunningInfo = NULL;
    addButton = NULL;
    autoAddIconTimer = NULL;
    m_cameraSearchHeading = NULL;

    autoAddIconTimer = new QTimer();
    connect(autoAddIconTimer,
            SIGNAL(timeout()),
            this,
            SLOT(slotautoAddIconTimeOut()));
    autoAddIconTimer->setInterval (1000);

    m_cameraSearchHeading = new TextLabel((SCALE_WIDTH(477)),
                                      (SCALE_HEIGHT(19)),
                                      SCALE_FONT(SUB_HEADING_FONT_SIZE),
                                      subHeadStr,
                                      this,
                                      HIGHLITED_FONT_COLOR,
                                      NORMAL_FONT_FAMILY,
                                      ALIGN_START_X_CENTRE_Y,
                                      0,
                                      false,
                                      CAM_SERCH_BGTILE_WIDTH,
                                      0);

    fieldsHeading[0] = new TableCell(SCALE_WIDTH(12),
                                     SCALE_HEIGHT(60),
                                     (SCALE_WIDTH(35) - 1),
                                     CAM_SEARCH_HEADER_CELL_HEIGHT,
                                     this,
                                     true);

    selectAllCam = new OptionSelectButton(fieldsHeading[0]->x () + SCALE_WIDTH(7),
                                          fieldsHeading[0]->y (),
                                          fieldsHeading[0]->width (),
                                          fieldsHeading[0]->height (),
                                          CHECK_BUTTON_INDEX,
                                          this,
                                          NO_LAYER,
                                          "","", -1,
                                          CAM_SRCH_SEL_ALL_CAM,
                                          false);
    selectAllCam->setIsEnabled(false);
    connect (selectAllCam,
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotOptionSelectionButton(OPTION_STATE_TYPE_e,int)));

    for(quint8 index = 1 ; index < MAX_CAM_SEARCH_FEILDS ; index++)
    {
        fieldsHeading[index] = new TableCell(fieldsHeading[index -1]->x () +
                                             fieldsHeading[index -1]->width (),
                                             fieldsHeading[index -1]->y (),
                                             (SCALE_WIDTH(headerWidthArray[index]) - 1),
                                             CAM_SEARCH_HEADER_CELL_HEIGHT,
                                             this,
                                             true);

        fieldsHeadingStr[index-1] = new TextLabel(fieldsHeading[index]->x () + SCALE_WIDTH(10),
                                                  fieldsHeading[index]->y () + (fieldsHeading[index]->height()/2),
                                                  NORMAL_FONT_SIZE,
                                                  cameraSearchStr[index],
                                                  this,
                                                  BLUE_COLOR,
                                                  NORMAL_FONT_FAMILY,
                                                  ALIGN_START_X_CENTRE_Y, 0, false,
                                                  (fieldsHeading[index]->width() - SCALE_WIDTH(8)));
    }

    srNumber[0] = new TableCell(fieldsHeading[0]->x (),
                                fieldsHeading[0]->y () +
                                fieldsHeading[0]->height (),
                                fieldsHeading[0]->width (),
                                CAM_SEARCH_TABLE_CELL_HEIGHT,
                                this);

    selectCam[0] = new OptionSelectButton(srNumber[0]->x () + SCALE_WIDTH(7),
                                          srNumber[0]->y (),
                                          srNumber[0]->width (),
                                          CAM_SEARCH_TABLE_CELL_HEIGHT,
                                          CHECK_BUTTON_INDEX,
                                          this,
                                          NO_LAYER,
                                          "","", -1,
                                          CAM_SRCH_SEL_CAM,
                                          true);
    selectCam[0]->setVisible (false);
    selectCam[0]->setIsEnabled(false);
    connect (selectCam[0],
             SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
             this,
             SLOT(slotOptionSelectionButton(OPTION_STATE_TYPE_e,int)));

    ipAddressCell[0] = new TableCell(fieldsHeading[1]->x (),
                                     fieldsHeading[1]->y () +
                                     fieldsHeading[1]->height (),
                                     fieldsHeading[1]->width (),
                                     CAM_SEARCH_TABLE_CELL_HEIGHT,
                                     this);

    ipv4AddressStr[0] = new TextLabel(ipAddressCell[0]->x () + SCALE_WIDTH(10),
                                      ipAddressCell[0]->y () +
                                      (ipAddressCell[0]->height ())/4,
                                      NORMAL_FONT_SIZE,
                                      "",
                                      this,
                                      NORMAL_FONT_COLOR,
                                      NORMAL_FONT_FAMILY,
                                      ALIGN_START_X_CENTRE_Y, 0, false,
                                      fieldsHeading[1]->width ());

    ipv6AddressStr[0] = new TextLabel(ipAddressCell[0]->x () + SCALE_WIDTH(10),
                                      ipAddressCell[0]->y () +
                                      (ipAddressCell[0]->height () * 3)/4,
                                      NORMAL_FONT_SIZE,
                                      "",
                                      this,
                                      NORMAL_FONT_COLOR,
                                      NORMAL_FONT_FAMILY,
                                      ALIGN_START_X_CENTRE_Y, 0, false,
                                      fieldsHeading[1]->width () - 10);

    httpPorts[0] = new TableCell(fieldsHeading[2]->x (),
                                 fieldsHeading[2]->y () +
                                 fieldsHeading[2]->height (),
                                 fieldsHeading[2]->width (),
                                 CAM_SEARCH_TABLE_CELL_HEIGHT,
                                 this);

    httpPortsStr[0] = new TextLabel(httpPorts[0]->x () + SCALE_WIDTH(10),
                                    httpPorts[0]->y () +
                                    (httpPorts[0]->height ())/2,
                                    NORMAL_FONT_SIZE,
                                    "",
                                    this,
                                    NORMAL_FONT_COLOR,
                                    NORMAL_FONT_FAMILY,
                                    ALIGN_START_X_CENTRE_Y);

    brands[0] = new TableCell(fieldsHeading[3]->x (),
                              fieldsHeading[3]->y () +
                              fieldsHeading[3]->height (),
                              fieldsHeading[3]->width (),
                              CAM_SEARCH_TABLE_CELL_HEIGHT,
                              this);

    brandsStr[0] = new TextLabel(brands[0]->x () + SCALE_WIDTH(10),
                                 brands[0]->y () +
                                 (brands[0]->height ())/2,
                                 NORMAL_FONT_SIZE,
                                 "",
                                 this,
                                 NORMAL_FONT_COLOR,
                                 NORMAL_FONT_FAMILY,
                                 ALIGN_START_X_CENTRE_Y);

    model[0] = new TableCell(fieldsHeading[4]->x (),
                             fieldsHeading[4]->y () +
                             fieldsHeading[4]->height (),
                             fieldsHeading[4]->width (),
                             CAM_SEARCH_TABLE_CELL_HEIGHT,
                             this);

    modelStr[0] = new TextLabel(model[0]->x () + SCALE_WIDTH(10),
                                model[0]->y () +
                                (model[0]->height ())/2,
                                NORMAL_FONT_SIZE,
                                "",
                                this,
                                NORMAL_FONT_COLOR,
                                NORMAL_FONT_FAMILY,
                                ALIGN_START_X_CENTRE_Y);

    onvifPorts[0] = new TableCell(fieldsHeading[5]->x (),
                                  fieldsHeading[5]->y () +
                                  fieldsHeading[5]->height (),
                                  fieldsHeading[5]->width (),
                                  CAM_SEARCH_TABLE_CELL_HEIGHT,
                                  this);

    onvifPortStr[0] = new TextLabel(onvifPorts[0]->x () + SCALE_WIDTH(10),
                                    onvifPorts[0]->y () +
                                    (onvifPorts[0]->height ())/2,
                                    NORMAL_FONT_SIZE,
                                    "",
                                    this,
                                    NORMAL_FONT_COLOR,
                                    NORMAL_FONT_FAMILY,
                                    ALIGN_START_X_CENTRE_Y);

    addCameraCell[0] = new TableCell(fieldsHeading[6]->x (),
                                     fieldsHeading[6]->y () +
                                     fieldsHeading[6]->height (),
                                     fieldsHeading[6]->width (),
                                     CAM_SEARCH_TABLE_CELL_HEIGHT,
                                     this);

    addCameraBtn[0] = new ControlButton(ADD_BUTTON_TABLE_INDEX,
                                        addCameraCell[0]->x () + SCALE_WIDTH(10),
                                        addCameraCell[0]->y (),
                                        addCameraCell[0]->width (),
                                        addCameraCell[0]->height (),
                                        this,
                                        NO_LAYER,
                                        -1,
                                        "",
                                        true,
                                        CAM_SRCH_ADD_CAM);
    connect (addCameraBtn[0],
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotAddButtonClick(int)));
    addCameraBtn[0]->setVisible (false);
    addCameraBtn[0]->setIsEnabled(false);

    testCamera[0] = new TableCell(fieldsHeading[7]->x (),
                                  fieldsHeading[7]->y () +
                                  fieldsHeading[7]->height (),
                                  fieldsHeading[7]->width (),
                                  CAM_SEARCH_TABLE_CELL_HEIGHT,
                                  this);

    testCameraBtn[0] = new ControlButton(TEST_CAMERAS_BUTTON_INDEX,
                                         testCamera[0]->x () + SCALE_WIDTH(15),
                                         testCamera[0]->y (),
                                         testCamera[0]->width (),
                                         testCamera[0]->height (),
                                         this,
                                         NO_LAYER,
                                         -1,
                                         "",
                                         true,
                                         (CAM_SRCH_TEST_CAM));
    connect (testCameraBtn[0],
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotTestButtonClick(int)));

    addCameraBtn[0]->setVisible (false);
    testCameraBtn[0]->setVisible (false);
    testCameraBtn[0]->setIsEnabled(false);

    for(quint8 index = 1 ; index < MAX_CAM_SEARCH_RECORD_DATA; index++)
    {
        srNumber[index] = new TableCell(srNumber[(index -1)]->x (),
                                        srNumber[(index -1)]->y () +
                                        srNumber[(index -1)]->height (),
                                        srNumber[(index -1)]->width () - 1,
                                        CAM_SEARCH_TABLE_CELL_HEIGHT,
                                        this);

        selectCam[index] = new OptionSelectButton(srNumber[index]->x () + SCALE_WIDTH(7),
                                                  srNumber[index]->y (),
                                                  srNumber[index]->width (),
                                                  srNumber[index]->height (),
                                                  CHECK_BUTTON_INDEX,
                                                  this,
                                                  NO_LAYER,
                                                  "","", -1,
                                                  (CAM_SRCH_SEL_CAM + (index * MAX_NAVIGATE_CONTROL_IN_ROW)),
                                                  true);
        selectCam[index]->setVisible (false);
        selectCam[index]->setIsEnabled(false);
        connect (selectCam[index],
                 SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                 this,
                 SLOT(slotOptionSelectionButton(OPTION_STATE_TYPE_e,int)));

        ipAddressCell[index] = new TableCell(ipAddressCell[(index -1)]->x (),
        									 ipAddressCell[(index -1)]->y () +
        									 ipAddressCell[(index -1)]->height (),
                                           	 ipAddressCell[(index -1)]->width () - 1,
                                           	 CAM_SEARCH_TABLE_CELL_HEIGHT,
                                           	 this);

        ipv4AddressStr[index] = new TextLabel(ipAddressCell[index]->x () + SCALE_WIDTH(10),
                                              ipAddressCell[index]->y () +
                                              (ipAddressCell[index]->height ())/4,
                                              NORMAL_FONT_SIZE,
                                              "",
                                              this,
                                              NORMAL_FONT_COLOR,
                                              NORMAL_FONT_FAMILY,
                                              ALIGN_START_X_CENTRE_Y, 0, false,
                                              ipAddressCell[(index -1)]->width () - 1);

        ipv6AddressStr[index] = new TextLabel(ipAddressCell[index]->x () + SCALE_WIDTH(10),
                                              ipAddressCell[index]->y () +
                                              (ipAddressCell[index]->height () * 3)/4,
                                              NORMAL_FONT_SIZE,
                                              "",
                                              this,
                                              NORMAL_FONT_COLOR,
                                              NORMAL_FONT_FAMILY,
                                              ALIGN_START_X_CENTRE_Y, 0, false,
                                              ipAddressCell[(index -1)]->width () - 10);

        httpPorts[index] = new TableCell(httpPorts[(index -1)]->x (),
                                         httpPorts[(index -1)]->y () +
                                         httpPorts[(index -1)]->height (),
                                         httpPorts[(index -1)]->width () - 1,
                                         CAM_SEARCH_TABLE_CELL_HEIGHT,
                                         this);

        httpPortsStr[index] = new TextLabel(httpPorts[index]->x () + SCALE_WIDTH(10),
                                            httpPorts[index]->y () +
                                            (httpPorts[index]->height ())/2,
                                            NORMAL_FONT_SIZE,
                                            "",
                                            this,
                                            NORMAL_FONT_COLOR,
                                            NORMAL_FONT_FAMILY,
                                            ALIGN_START_X_CENTRE_Y);

        brands[index] = new TableCell(brands[(index -1)]->x (),
                                      brands[(index -1)]->y () +
                                      brands[(index -1)]->height (),
                                      brands[(index -1)]->width () - 1,
                                      CAM_SEARCH_TABLE_CELL_HEIGHT,
                                      this);

        brandsStr[index] = new TextLabel(brands[index]->x () + SCALE_WIDTH(10),
                                         brands[index]->y () +
                                         (brands[index]->height ())/2,
                                         NORMAL_FONT_SIZE,
                                         "",
                                         this,
                                         NORMAL_FONT_COLOR,
                                         NORMAL_FONT_FAMILY,
                                         ALIGN_START_X_CENTRE_Y);

        model[index] = new TableCell(model[(index -1)]->x (),
                                     model[(index -1)]->y () +
                                     model[(index -1)]->height (),
                                     model[(index -1)]->width () - 1,
                                     CAM_SEARCH_TABLE_CELL_HEIGHT,
                                     this);

        modelStr[index] = new TextLabel(model[index]->x () + SCALE_WIDTH(10),
                                        model[index]->y () +
                                        (model[index]->height ())/2,
                                        NORMAL_FONT_SIZE,
                                        "",
                                        this,
                                        NORMAL_FONT_COLOR,
                                        NORMAL_FONT_FAMILY,
                                        ALIGN_START_X_CENTRE_Y);

        onvifPorts[index] = new TableCell(onvifPorts[(index - 1)]->x (),
                                          onvifPorts[(index - 1)]->y () +
                                          onvifPorts[(index - 1)]->height (),
                                          onvifPorts[(index - 1)]->width () - 1,
                                          CAM_SEARCH_TABLE_CELL_HEIGHT,
                                          this);

        onvifPortStr[index] = new TextLabel(onvifPorts[index]->x () + SCALE_WIDTH(10),
                                            onvifPorts[index]->y () +
                                            (onvifPorts[index]->height ())/2,
                                            NORMAL_FONT_SIZE,
                                            "",
                                            this,
                                            NORMAL_FONT_COLOR,
                                            NORMAL_FONT_FAMILY,
                                            ALIGN_START_X_CENTRE_Y);

        addCameraCell[index] = new TableCell(addCameraCell[(index -1)]->x (),
                                             addCameraCell[(index -1)]->y () +
                                             addCameraCell[(index -1)]->height (),
                                             addCameraCell[(index -1)]->width () - 1,
                                             CAM_SEARCH_TABLE_CELL_HEIGHT,
                                             this);

        addCameraBtn[index] = new ControlButton(ADD_BUTTON_TABLE_INDEX,
                                                addCameraCell[index]->x () + SCALE_WIDTH(10),
                                                addCameraCell[index]->y (),
                                                addCameraCell[index]->width (),
                                                addCameraCell[index]->height (),
                                                this,
                                                NO_LAYER,
                                                -1,
                                                "",
                                                true,
                                                (index*MAX_NAVIGATE_CONTROL_IN_ROW + CAM_SRCH_ADD_CAM));
        connect (addCameraBtn[index],
                 SIGNAL(sigButtonClick(int)),
                 this,
                 SLOT(slotAddButtonClick(int)));
        addCameraBtn[index]->setVisible (false);
        addCameraBtn[index]->setIsEnabled(false);

        testCamera[index] = new TableCell(testCamera[(index -1)]->x (),
                                          testCamera[(index -1)]->y () +
                                          testCamera[(index -1)]->height (),
                                          testCamera[(index -1)]->width ()- 1,
                                          CAM_SEARCH_TABLE_CELL_HEIGHT,
                                          this);

        testCameraBtn[index] = new ControlButton(TEST_CAMERAS_BUTTON_INDEX,
                                                 testCamera[index]->x () + SCALE_WIDTH(15),
                                                 testCamera[index]->y (),
                                                 testCamera[index]->width (),
                                                 testCamera[index]->height (),
                                                 this,
                                                 NO_LAYER,
                                                 -1,
                                                 "",
                                                 true,
                                                 (index*MAX_NAVIGATE_CONTROL_IN_ROW + CAM_SRCH_TEST_CAM));
        connect (testCameraBtn[index],
                 SIGNAL(sigButtonClick(int)),
                 this,
                 SLOT(slotTestButtonClick(int)));
        testCameraBtn[index]->setVisible (false);
        testCameraBtn[index]->setIsEnabled(false);
    }

    previousButton = new ControlButton(PREVIOUS_BUTTON_INDEX,
                                       SCALE_WIDTH(35),
                                       SCALE_HEIGHT(520),
                                       SCALE_WIDTH(35),
                                       BGTILE_HEIGHT,
                                       this,
                                       NO_LAYER,
                                       -1,
                                       cameraSearchStr[MAX_CAM_SEARCH_FEILDS],
                                       false,
                                       CAM_SRCH_PREVIOUS_BUTTON,
                                       false);
    connect (previousButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    addButton = new CnfgButton(CNFGBUTTON_MEDIAM,
                               SCALE_WIDTH(530),
                               SCALE_HEIGHT(540),
                               "Add",
                               this,
                               CAM_SRCH_AUTO_ADD,
                               false);
    addButton->setIsEnabled(false);
    connect (addButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    nextButton = new ControlButton(NEXT_BUTTON_INDEX,
                                   SCALE_WIDTH(970),
                                   SCALE_HEIGHT(520),
                                   SCALE_WIDTH(970),
                                   BGTILE_HEIGHT,
                                   this,
                                   NO_LAYER,
                                   -1,
                                   cameraSearchStr[MAX_CAM_SEARCH_FEILDS + 3],
                                   false,
                                   CAM_SRCH_NEXT_BUTTON);
    connect (nextButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    infoPage = new InfoPage (0, 0,
                             SCALE_WIDTH(1145),
                             SCALE_HEIGHT(750),
                             MAX_INFO_PAGE_TYPE,
                             parentWidget());
    connect (infoPage,
             SIGNAL(sigInfoPageCnfgBtnClick(int)),
             this,
             SLOT(slotInfoPageBtnclick(int)));
}

SearchCamera::~SearchCamera ()
{
    WizardCommon:: UnloadProcessBar();
    cancelCamAutoSearchCmd();
    if(IS_VALID_OBJ(selectAllCam))
    {
        disconnect (selectAllCam,
                    SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                    this,
                    SLOT(slotOptionSelectionButton(OPTION_STATE_TYPE_e,int)));
        delete selectAllCam;
    }

    for(quint8 index = 0 ; index < MAX_CAM_SEARCH_FEILDS ; index++)
    {
        DELETE_OBJ(fieldsHeading[index]);
        if (index < (MAX_CAM_SEARCH_FEILDS - 1))
        {
            DELETE_OBJ(fieldsHeadingStr[index]);
        }
    }

    for(quint8 index = 0 ; index < MAX_CAM_SEARCH_RECORD_DATA; index++)
    {
        DELETE_OBJ(srNumber[index]);
        DELETE_OBJ(ipAddressCell[index]);
        DELETE_OBJ(ipv4AddressStr[index]);
        DELETE_OBJ(ipv6AddressStr[index]);
        DELETE_OBJ(httpPorts[index]);
        DELETE_OBJ(httpPortsStr[index]);
        DELETE_OBJ(brands[index]);
        DELETE_OBJ(brandsStr[index]);
        DELETE_OBJ(model[index]);
        DELETE_OBJ(modelStr[index]);
        DELETE_OBJ(onvifPorts[index]);
        DELETE_OBJ(onvifPortStr[index]);
        DELETE_OBJ(addCameraCell[index]);

        if(IS_VALID_OBJ(selectCam[index]))
        {
            disconnect (selectCam[index],
                        SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                        this,
                        SLOT(slotOptionSelectionButton(OPTION_STATE_TYPE_e,int)));
            delete selectCam[index];
        }

        if(IS_VALID_OBJ(addCameraBtn[index]))
        {
            disconnect (addCameraBtn[index],
                        SIGNAL(sigButtonClick(int)),
                        this,
                        SLOT(slotAddButtonClick(int)));
            delete addCameraBtn[index] ;
        }

        if(IS_VALID_OBJ(testCameraBtn[index]))
        {
            disconnect (testCameraBtn[index],
                        SIGNAL(sigButtonClick(int)),
                        this,
                        SLOT(slotTestButtonClick(int)));
            delete testCameraBtn[index];
        }

        DELETE_OBJ(testCamera[index]);
    }

    if(IS_VALID_OBJ(previousButton))
    {
        disconnect (previousButton,
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotButtonClick(int)));
        delete previousButton;
    }

    if(IS_VALID_OBJ(addButton))
    {
        disconnect (addButton,
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotButtonClick(int)));
        delete addButton;
    }

    if(IS_VALID_OBJ(nextButton))
    {
        disconnect (nextButton,
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotButtonClick(int)));
        delete nextButton;
    }

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

    if(IS_VALID_OBJ(getAcqListTimer))
    {
        disconnect (getAcqListTimer,
                    SIGNAL(timeout()),
                    this,
                    SLOT(slotGetAcqListTimerTimeout()));
        delete getAcqListTimer;
    }

    WizardCommon:: UnloadProcessBar();
    DELETE_OBJ(payloadLib);
    if(autoAddIconTimer != NULL)
    {
       disconnect (autoAddIconTimer,
                SIGNAL(timeout()),
                this,
                SLOT(slotautoAddIconTimeOut()));
       DELETE_OBJ(autoAddIconTimer);
    }

    DELETE_OBJ(m_cameraSearchHeading);
    DELETE_OBJ(infoPage);
}

void SearchCamera::updateControlsforSearch (bool )
{
    testCameraIndex = INVALID_CAMERA_INDEX;

    addButton->setIsEnabled((isUpdateBeforeSearch) ? false : (ipAddressList.empty()) ? false : true);
    if(isUpdateBeforeSearch)
    {
        clearAllList ();
        showCameraSearchList ();
        selectAllCam->setIsEnabled (false);
        autoAddIconTimer->stop ();
        autoAddIpList.clear ();
    }
    else
    {
        if((!ipAddressList.empty()) && (addButton->getIsEnabled ()))
        {
            if(!autoAddIconTimer->isActive ())
            {
                autoAddIconTimer->start ();
            }
        }
    }
}

void SearchCamera::clearAllList ()
{
    searchList.clear();
    camStatusList.clear();
    camNumList.clear();
    camNameList.clear();
    ipAddressList.clear();
    brandList.clear();
    modelList.clear();
    onvifSupportStatusList.clear();
    onvifPortList.clear();
    httpPortList.clear();
}

void SearchCamera::loadInfoPageMsg (QString infoMsg)
{
    autoAddIconTimer->stop ();
    WizardCommon :: InfoPageImage();
    isInfoPageLoaded = true;
    infoPage->raise();
    infoPage->loadInfoPage (infoMsg);
}

void SearchCamera::sendCamAutoSearchCmd()
{
    if(cameraSearchProcess == NULL)
    {
        cameraSearchProcess = new CameraSearchProcess(SCALE_WIDTH(50),
                                                      SCALE_HEIGHT(40),
                                                      SCALE_WIDTH(50),
                                                      SCALE_HEIGHT(10),
                                                      this);
    }

    isUpdateBeforeSearch = true;
    isCancelSend = false;
    updateControlsforSearch(true);
    currentPageNo = 0;
    isAdvanceCameraSearchRunning = false;

    // sending 0 which indicates that command is for search page
    payloadLib->setCnfgArrayAtIndex (0,0);
    sendCommand(AUTO_SEARCH,1);
}

void SearchCamera::showCameraSearchList()
{
    quint8 recordOnPage = 0;
    quint8 eleIndex = 0;

    selectAllCam->setVisible (true);
    maxSearchListCount = ipAddressList.length ();
    maximumPages = (maxSearchListCount % MAX_CAM_SEARCH_RECORD_DATA == 0 ) ?
                (maxSearchListCount / MAX_CAM_SEARCH_RECORD_DATA) : ((maxSearchListCount / MAX_CAM_SEARCH_RECORD_DATA) + 1);
    if(maxSearchListCount < (MAX_CAM_SEARCH_RECORD_DATA*(currentPageNo + 1)))
    {
        recordOnPage = maxSearchListCount - ((MAX_CAM_SEARCH_RECORD_DATA*(currentPageNo)) );
    }
    else
    {
        recordOnPage = MAX_CAM_SEARCH_RECORD_DATA;
    }

    if(((recordOnPage == 0) && (maxSearchListCount != 0)) || (maximumPages <= currentPageNo))
    {
        currentPageNo = 0;
        if(maxSearchListCount < (MAX_CAM_SEARCH_RECORD_DATA*(currentPageNo + 1)))
        {
            recordOnPage = maxSearchListCount - ((MAX_CAM_SEARCH_RECORD_DATA*(currentPageNo)) );
        }
        else
        {
            recordOnPage = MAX_CAM_SEARCH_RECORD_DATA;
        }
    }

    if(recordOnPage != 0)
    {
        updateNavigationControlStatus();
    }
    else
    {
        previousButton->setIsEnabled (false);
        nextButton->setIsEnabled (false);
        currentPageNo = 0;
    }

    for(quint8 index = 0; index < recordOnPage; index++)
    {
        eleIndex = ((index + (currentPageNo*MAX_CAM_SEARCH_RECORD_DATA)));
        selectCam[index]->setVisible (true);

        if (ipAddressList[eleIndex].first.isEmpty())
        {
             ipv6AddressStr[index]->resetGeometry(ipAddressCell[index]->x() + SCALE_WIDTH(10) , ipAddressCell[index]->y() + ipAddressCell[index]->height()/2);
             ipv6AddressStr[index]->changeText (ipAddressList[eleIndex].second);
             ipv4AddressStr[index]->changeText("");
             ipv4AddressStr[index]->setVisible(false);
             ipv6AddressStr[index]->setVisible(true);
        }
        else if (ipAddressList[eleIndex].second.isEmpty())
        {
            ipv4AddressStr[index]->resetGeometry(ipAddressCell[index]->x() + SCALE_WIDTH(10), ipAddressCell[index]->y() + ipAddressCell[index]->height()/2);
            ipv4AddressStr[index]->changeText (ipAddressList[eleIndex].first);
            ipv6AddressStr[index]->changeText("");
            ipv6AddressStr[index]->setVisible(false);
            ipv4AddressStr[index]->setVisible(true);
        }
        else
        {
            ipv4AddressStr[index]->resetGeometry(ipAddressCell[index]->x() + SCALE_WIDTH(10), ipAddressCell[index]->y() + ipAddressCell[index]->height()/4);
            ipv6AddressStr[index]->resetGeometry(ipAddressCell[index]->x() + SCALE_WIDTH(10),ipAddressCell[index]->y() + (ipAddressCell[index]->height() * 3)/4);
            ipv4AddressStr[index]->changeText (ipAddressList[eleIndex].first);
            ipv6AddressStr[index]->changeText (ipAddressList[eleIndex].second);
            ipv4AddressStr[index]->setVisible(true);
            ipv6AddressStr[index]->setVisible(true);
        }
        ipv4AddressStr[index]->update();
        ipv6AddressStr[index]->update();

        httpPortsStr[index]->changeText (httpPortList.at (eleIndex));
        httpPortsStr[index]->update ();

        brandsStr[index]->changeText (brandList.at (eleIndex));
        brandsStr[index]->update ();

        modelStr[index]->changeText (modelList.at (eleIndex));
        modelStr[index]->update ();

        (onvifSupportStatusList[eleIndex] == "0") ? onvifPortStr[index]->changeText (DEFAULT_FIELD_VALUE_STR) :
                                                    onvifPortStr[index]->changeText (onvifPortList.at (eleIndex));
        onvifPortStr[index]->update ();

        addCameraBtn[index]->setVisible (true);
        addCameraBtn[index]->setIsEnabled (true);

        addCameraBtn[index]->changeImageType(((MX_CAM_STATUS_e)camStatusList.at (eleIndex).toUInt () == MX_CAM_ADDED)
                                             ? ADDED_BUTTON_TABLE_INDEX : ADD_BUTTON_TABLE_INDEX);
        addCameraBtn[index]->update ();

        testCameraBtn[index]->setIsEnabled((((MX_CAM_STATUS_e)camStatusList.at (eleIndex).toUInt () == MX_CAM_ADDED) ? true : false));
        testCameraBtn[index]->setVisible (true);
    }

    if(recordOnPage != MAX_CAM_SEARCH_RECORD_DATA)
    {
        clearSerachDisplayList(recordOnPage);
    }
}

void SearchCamera::slotGetAcqListTimerTimeout()
{
    if(isCancelSend == false)
    {
        sendCommand (GET_ACQ_LIST);
    }
}

void SearchCamera::slotOptionSelectionButton (OPTION_STATE_TYPE_e state, int indexInPage)
{
    MX_CAM_STATUS_e camState = MX_CAM_UNIDENTIFY;
    quint8 recordOnPage = 0;
    quint8 eleIndex = 0;
    quint8 numberOfDisable = 0;
    quint8 numberOfOnState = 0;
    QPair<QString, QString> ipAddrPair;

    if(maxSearchListCount < (MAX_CAM_SEARCH_RECORD_DATA*(currentPageNo + 1)))
    {
        recordOnPage = maxSearchListCount - ((MAX_CAM_SEARCH_RECORD_DATA*(currentPageNo)) );
    }
    else
    {
        recordOnPage = MAX_CAM_SEARCH_RECORD_DATA;
    }

    if(indexInPage == CAM_SRCH_SEL_ALL_CAM)
    {
        if(state == ON_STATE)
        {
            updateSelCamState (true);
        }
        else
        {
            for(quint8 index = 0; index < recordOnPage; index++)
            {
                selectCam[index]->changeState (OFF_STATE);
                eleIndex = ((index) + (currentPageNo*(MAX_CAM_SEARCH_RECORD_DATA)));
                autoAddIpList.removeOne(ipAddressList[eleIndex]);
            }
        }
    }
    else
    {
        for(quint8 index = 0; index < recordOnPage; index++)
        {
            eleIndex = ((index) + (currentPageNo*(MAX_CAM_SEARCH_RECORD_DATA)));
            camState =(MX_CAM_STATUS_e)camStatusList.at(eleIndex).toUInt ();

            if(camState == MX_CAM_ADDED)
            {
                numberOfDisable++;
            }
            else if((onvifSupportStatusList.at (eleIndex).toUInt () == 0) && (camState == MX_CAM_UNIDENTIFY))
            {
                numberOfDisable++;
            }
            else
            {
                if(selectCam[index]->getCurrentState () == ON_STATE)
                {
                    numberOfOnState++;
                }
            }
        }

        if(((recordOnPage - numberOfOnState) == numberOfDisable) && (numberOfDisable != recordOnPage))
        {
            selectAllCam->changeState (ON_STATE);
        }
        else
        {
            selectAllCam->changeState (OFF_STATE);
        }

        if(state == OFF_STATE)
        {
            quint8 index = ((indexInPage/MAX_NAVIGATE_CONTROL_IN_ROW) - 1);
            ipAddrPair.first = ipv4AddressStr[index]->getText();
            ipAddrPair.second = ipv6AddressStr[index]->getText();
            if(autoAddIpList.contains (ipAddrPair))
            {
                autoAddIpList.removeOne (ipAddrPair);
            }
        }
    }
}

void SearchCamera::slotAddButtonClick (int indexInPage)
{
    quint8 index = ((indexInPage/MAX_NAVIGATE_CONTROL_IN_ROW) - 1);

    index = ((index) + (currentPageNo*(MAX_CAM_SEARCH_RECORD_DATA)));

    currentBrandName = brandList.at (index);
    currentModelName = modelList.at (index);
    currentCameraName = camNameList.at (index);

    if(addCamera == NULL)
    {
        isBlockListFlag = true;
        WizardCommon:: InfoPageImage();
        quint8 mapIndex = 0;
        QMap<quint8, QString> ipAddrListMap;

        if (!ipAddressList[index].first.isEmpty())
        {
            ipAddrListMap.insert(mapIndex++, ipAddressList[index].first);
        }

        if (!ipAddressList[index].second.isEmpty())
        {
            ipAddrListMap.insert(mapIndex++, ipAddressList[index].second);
        }

        addCamera = new WizardAddCamera(currDevName,
                                        ipAddrListMap,
                                        httpPortList.at (index),
                                        onvifPortList.at (index),
                                        onvifSupportStatusList.at (index).toUInt (),
                                        currentBrandName,
                                        currentModelName,
                                        currentCameraName,
                                        username,
                                        password,
                                        parentWidget (),
                                        index,
                                        (MX_CAM_STATUS_e)camStatusList.at(index).toUInt (),
                                        (quint8)camNumList.at(index).toUInt (),
                                        &devTableInfo);
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

void SearchCamera::slotTestCamDelete(quint8)
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
        delete testCam;
        testCam = NULL;
        WizardCommon:: UnloadInfoPageImage();
        infoPage->raise();
    }

    if(!autoAddIconTimer->isActive ())
    {
        autoAddIconTimer->start ();
    }
}

void SearchCamera::slotAddCameraDelete (quint8 cameraIndex,
                                        bool saveCameraFlag,
                                        QString ipAddressString,
                                        QString httpPortStr,
                                        QString tOnvifPortStr,
                                        bool tOnvifSupport,
                                        QString brandlistStr,
                                        QString modellistStr,
                                        QString camName,
                                        QString userName,
                                        QString tPassword,
                                        quint8  selIndex,
                                        quint8  currentIndex)
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
        delete addCamera;
        addCamera = NULL;
        isBlockListFlag = false;
        WizardCommon::UnloadInfoPageImage();
    }

    if(!autoAddIconTimer->isActive ())
    {
        autoAddIconTimer->start ();
    }

    if(saveCameraFlag)
    {
        requestedIp = ipAddressString;
        requestedPort = tOnvifPortStr;
        currentCameraName = camName;
        this->username = userName;
        this->password = tPassword;

        onvifSupportStatusList.replace (cameraIndex,QString("%1").arg (tOnvifSupport));

        payloadLib->setCnfgArrayAtIndex (0, selIndex);
        payloadLib->setCnfgArrayAtIndex (1, camName);
        payloadLib->setCnfgArrayAtIndex (2, ipAddressString);
        payloadLib->setCnfgArrayAtIndex (3, httpPortStr);

        if(tOnvifSupport)
        {
            payloadLib->setCnfgArrayAtIndex (4, "");
            payloadLib->setCnfgArrayAtIndex (5, "");
        }
        else
        {
            payloadLib->setCnfgArrayAtIndex (4, brandlistStr);
            payloadLib->setCnfgArrayAtIndex (5, modellistStr);
            brandList.replace(cameraIndex, brandlistStr);
            modelList.replace(cameraIndex, modellistStr);
        }

        payloadLib->setCnfgArrayAtIndex (6, userName);
        payloadLib->setCnfgArrayAtIndex (7, tPassword);
        payloadLib->setCnfgArrayAtIndex (8, ((tOnvifSupport == true) ? 1 : 0));
        payloadLib->setCnfgArrayAtIndex (9, tOnvifPortStr);
        sendCommand(ADD_CAMERAS, 10);
    }
}

void SearchCamera::slotTestButtonClick (int indexInPage)
{
    processBar->loadProcessBar();
    quint8 index = ((indexInPage/MAX_NAVIGATE_CONTROL_IN_ROW) - 1);

    index = ((index) + (currentPageNo*(MAX_CAM_SEARCH_RECORD_DATA)));
    autoAddIconTimer->stop ();
    testCameraIndex = index;
    payloadLib->setCnfgArrayAtIndex (0, camNumList.at(index));
    sendCommand (TST_CAM, 1);
}

void SearchCamera::clearSerachDisplayList (quint8 recordOnPage)
{
    for(quint8 index = recordOnPage; index < MAX_CAM_SEARCH_RECORD_DATA; index++)
    {
        ipv4AddressStr[index]->changeText ("");
        ipv6AddressStr[index]->changeText ("");
        httpPortsStr[index]->changeText ("");
        onvifPortStr[index]->changeText ("");
        brandsStr[index]->changeText ("");
        modelStr[index]->changeText ("");
        addCameraBtn[index]->setVisible (false);
        addCameraBtn[index]->setIsEnabled (false);  // for navigation must disable
        testCameraBtn[index]->setVisible (false);
        testCameraBtn[index]->setIsEnabled (false);  // for navigation must disable
        selectCam[index]->setVisible (false);
        selectCam[index]->setIsEnabled (false);  // for navigation must disable
    }
}

void SearchCamera::updateNavigationControlStatus ()
{
    previousButton->setIsEnabled ((currentPageNo != 0 ? true : false ));

    if( currentPageNo < (maximumPages - 1))
    {
        nextButton->setIsEnabled (true);
    }
    else if( currentPageNo == (maximumPages - 1 ))
    {
        nextButton->setIsEnabled (false);
    }
}

void SearchCamera::cancelCamAutoSearchCmd()
{
    if(IS_VALID_OBJ(getAcqListTimer))
    {
        if(getAcqListTimer->isActive())
        {
            getAcqListTimer->stop();
        }
    }
    isCancelSend = true;
    sendCommand(CNCL_AUTO_SEARCH);
}

void SearchCamera::updateSelCamState (bool isStateOn)
{
    bool autoAddStateOn = false;
    bool selEnable = false;
    quint8 eleIndex = 0;
    quint8 recordOnPage = 0;
    quint8 numberOfDisable = 0;
    quint8 numberOfOnState = 0;
    MX_CAM_STATUS_e camState;
    OPTION_STATE_TYPE_e selState;

    if(!camStatusList.empty ())
    {
        if(maxSearchListCount < (MAX_CAM_SEARCH_RECORD_DATA*(currentPageNo + 1)))
        {
            recordOnPage = maxSearchListCount - ((MAX_CAM_SEARCH_RECORD_DATA*(currentPageNo)) );
        }
        else
        {
            recordOnPage = MAX_CAM_SEARCH_RECORD_DATA;
        }

        for(quint8 index = 0; index < recordOnPage; index++)
        {
            autoAddStateOn = false;
            selState = OFF_STATE;
            selEnable = false;
            eleIndex = ((index) + (currentPageNo*(MAX_CAM_SEARCH_RECORD_DATA)));
            camState = (MX_CAM_STATUS_e)camStatusList.at(eleIndex).toUInt ();
            if((camState == MX_CAM_ADDED))
            {
                numberOfDisable++;
            }
            else
            {
                if((onvifSupportStatusList.at (eleIndex).toUInt () == 0) && (camState == MX_CAM_UNIDENTIFY))
                {
                    numberOfDisable++;
                }
                else
                {
                    selEnable = true;
                    if(!autoAddIpList.isEmpty ())
                    {
                        if(autoAddIpList.contains(QPair<QString, QString>(ipv4AddressStr[index]->getText(), ipv6AddressStr[index]->getText())))
                        {
                            autoAddStateOn = true;
                        }
                    }

                    if((isStateOn) || (autoAddStateOn))
                    {
                        selState = ON_STATE;
                        numberOfOnState++;
                    }
                }
            }

            selectCam[index]->changeState (selState);
            selectCam[index]->setIsEnabled (selEnable);
        }

        if(((recordOnPage - numberOfOnState) == numberOfDisable) && (numberOfDisable != recordOnPage))
        {
            selectAllCam->changeState (ON_STATE);
        }
        else
        {
            selectAllCam->changeState (OFF_STATE);
        }

        selectAllCam->setIsEnabled ((numberOfDisable == recordOnPage) ? false : true);
    }
}

void SearchCamera::slotButtonClick (int indexInPage)
{
    switch(indexInPage)
    {
        case CAM_SRCH_AUTO_ADD:
        {
            fillAutoAddIpList ();
            if(autoAddIpList.empty())
            {
                loadInfoPageMsg(ValidationMessage::getValidationMessage(CAM_SRCH_SELECT_CAM_TO_AUTO_CONFI));
            }
            else
            {
                sendCommand (MAX_ADD_CAM);
            }
        }
        break;

        case CAM_SRCH_PREVIOUS_BUTTON:
        {
            fillAutoAddIpList ();
            if(currentPageNo > 0)
            {
                currentPageNo --;
            }
            showCameraSearchList ();
            updateSelCamState();
        }
        break;

        case CAM_SRCH_NEXT_BUTTON:
        {
            fillAutoAddIpList ();
            if (currentPageNo != (maximumPages - 1))
            {
                currentPageNo ++;
            }
            showCameraSearchList ();
            updateSelCamState();
        }
        break;

        default:
            break;
    }
}

void SearchCamera::sendCommand(SET_COMMAND_e cmdType, quint32 totalfeilds)
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = cmdType;
    param->payload = payloadLib->createDevCmdPayload(totalfeilds);
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void SearchCamera::saveConfig()
{
    if(m_getConfig == true)
    {
        cancelCamAutoSearchCmd();
    }
}

void SearchCamera::updateList(DevCommParam *param)
{
    quint8 index = 0, totalResult = 0;
    QString camFoundStr;
    QString ipv4Addr, ipv6Addr;
    clearAllList();
    if(param->deviceStatus == CMD_SUCCESS)
    {
        payloadLib->parseDevCmdReply (true,param->payload);
        totalResult = (payloadLib->getTotalCmdFields () / MAX_MX_CMD_CAM_SEARCH_FIELDS);
        for(index = 0; index < totalResult; index++)
        {
            camStatusList.append (payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_CAMERA_STATUS + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString ());
            camNumList.append (payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_CAM_NUMBER + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString ());
            ipv4Addr = payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_IPV4_ADDRESS + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString();
            ipv6Addr = payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_IPV6_ADDRESS + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString();
            ipAddressList.append (QPair<QString, QString>(ipv4Addr, ipv6Addr));
            httpPortList.append (payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_HTTP_PORT + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString ());
            brandList.append (payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_BRAND_NAME + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString ());
            modelList.append (payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_MODEL_NAME + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString ());
            onvifSupportStatusList.append (payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_ONVIF_SUPPORT + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString ());
            onvifPortList.append (payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_ONVIF_PORT + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString ());
            QString camName = payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_CAM_NAME + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString ();

            if(camName == "")
            {
                if(payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_BRAND_NAME + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString () != "")
                {
                    camNameList.append (payloadLib->getCnfgArrayAtIndex(MX_CMD_FIELDS_BRAND_NAME + (index * MAX_MX_CMD_CAM_SEARCH_FIELDS)).toString ());
                }
                else if (!ipv4Addr.isEmpty())
                {
                    camNameList.append(ipv4Addr);
                }
                else
                {
                    camNameList.append(ipv6Addr);
                }
            }
            else
            {
                camNameList.append(camName);
            }
        }
    }

    maxSearchListCount = ipAddressList.length ();
    maximumPages = (maxSearchListCount % MAX_CAM_SEARCH_RECORD_DATA == 0 )
            ? (maxSearchListCount / MAX_CAM_SEARCH_RECORD_DATA) : ((maxSearchListCount / MAX_CAM_SEARCH_RECORD_DATA) + 1);
    showCameraSearchList ();
    camFoundStr = Multilang("Searching...") + QString(" ") + QString("%1").arg (ipAddressList.length ())+ QString(" ") + Multilang("camera(s) found");
    if((searchRunningInfo == NULL) && (isCancelSend == false))
    {
        searchRunningInfo = new TextLabel((SCALE_WIDTH(550)),
                                          SCALE_HEIGHT(30),
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
        searchRunningInfo->changeText (camFoundStr);
        searchRunningInfo->update ();
    }

    fillAutoAddIpList();
    updateSelCamState();
    addButton->setIsEnabled(((ipAddressList.empty ()) /*|| (currentFilterType == WIZ_FILTER_ADDED)*/)? false : true);

    if((!ipAddressList.empty ()) && (addButton->getIsEnabled ()))
    {
        if(!autoAddIconTimer->isActive ())
        {
            autoAddIconTimer->start ();
        }
    }
}

void SearchCamera::fillAutoAddIpList ()
{
    quint8 recordOnPage;

    if(maxSearchListCount < (MAX_CAM_SEARCH_RECORD_DATA*(currentPageNo + 1)))
    {
        recordOnPage = maxSearchListCount - ((MAX_CAM_SEARCH_RECORD_DATA*(currentPageNo)) );
    }
    else
    {
        recordOnPage = MAX_CAM_SEARCH_RECORD_DATA;
    }

    for(quint8 index = 0; index < recordOnPage; index++)
    {
        if((selectCam[index]->getIsEnabled ()) && (selectCam[index]->getCurrentState () == ON_STATE))
        {
            quint8 eleIndex = ((index) + (currentPageNo*(MAX_CAM_SEARCH_RECORD_DATA)));
            if(!autoAddIpList.contains (ipAddressList[eleIndex]))
            {
                autoAddIpList.append (ipAddressList[eleIndex]);
            }
        }
    }

    if(autoAddIpList.length() > ipAddressList.length())
    {
        quint8 temp = 0;
        while (autoAddIpList.length() > ipAddressList.length())
        {
            if(ipAddressList.contains(autoAddIpList[temp]) == false)
            {
                autoAddIpList.removeAt(temp);
            }
            else
            {
                temp++;
            }

            if (temp >= autoAddIpList.length())
            {
                break;
            }
        }
    }
}

void SearchCamera::sendAutoAddCmd ()
{
    qint16 eleIndex = 0;
    quint8 totalCam = autoAddIpList.length ();

    for(quint8 index = 0; index < totalCam; index++)
    {
        eleIndex = ipAddressList.indexOf(autoAddIpList[index]);
        if(eleIndex == -1)
        {
            continue;
        }

        QString tempCamName = camNameList.at (eleIndex);
        if(tempCamName.size() > MAX_CAMERA_NAME_LENGTH)
        {
            tempCamName.truncate(MAX_CAMERA_NAME_LENGTH);
        }

        payloadLib->setCnfgArrayAtIndex((MX_CMD_AUTO_ADD_FEILDS_CAM_STATUS + (MAX_MX_CMD_AUTO_ADD_FIELDS*index)), camStatusList.at(eleIndex));
        payloadLib->setCnfgArrayAtIndex((MX_CMD_AUTO_ADD_FEILDS_CAM_NAME + (MAX_MX_CMD_AUTO_ADD_FIELDS*index)), tempCamName);
        if ((retainIpAddrInAutoConfig == true) || (autoConfigIpAddrFamily == QAbstractSocket::IPv4Protocol))
        {
            payloadLib->setCnfgArrayAtIndex((MX_CMD_AUTO_ADD_FEILDS_IP_ADDRESS + (MAX_MX_CMD_AUTO_ADD_FIELDS*index)),
                                            ((ipAddressList[eleIndex].first.isEmpty() == false) ? ipAddressList[eleIndex].first : ipAddressList[eleIndex].second));
        }
        else
        {
            payloadLib->setCnfgArrayAtIndex((MX_CMD_AUTO_ADD_FEILDS_IP_ADDRESS + (MAX_MX_CMD_AUTO_ADD_FIELDS*index)),
                                            ((ipAddressList[eleIndex].second.isEmpty() == false) ? ipAddressList[eleIndex].second : ipAddressList[eleIndex].first));
        }
        payloadLib->setCnfgArrayAtIndex((MX_CMD_AUTO_ADD_FEILDS_HTTP_PORT + (MAX_MX_CMD_AUTO_ADD_FIELDS*index)), httpPortList.at (eleIndex));
        payloadLib->setCnfgArrayAtIndex((MX_CMD_AUTO_ADD_FEILDS_BRAND_NAME + (MAX_MX_CMD_AUTO_ADD_FIELDS*index)), brandList.at (eleIndex));
        payloadLib->setCnfgArrayAtIndex((MX_CMD_AUTO_ADD_FEILDS_MODEL_NAME + (MAX_MX_CMD_AUTO_ADD_FIELDS*index)), modelList.at (eleIndex));
        payloadLib->setCnfgArrayAtIndex((MX_CMD_AUTO_ADD_FEILDS_ONVIF_SUPPORT + (MAX_MX_CMD_AUTO_ADD_FIELDS*index)), onvifSupportStatusList.at (eleIndex));
        payloadLib->setCnfgArrayAtIndex((AUTO_ADD_FEILDS_ONVIF_PORT + (MAX_MX_CMD_AUTO_ADD_FIELDS*index)), onvifPortList.at (eleIndex));
    }

    autoAddIpList.clear ();
    sendCommand(AUTO_CONFIGURE_CAMERA, (MAX_MX_CMD_AUTO_ADD_FIELDS*totalCam));
}


void SearchCamera::getConfig ()
{
    QString payloadString = payloadLib->createDevCnfgPayload(MSG_GET_CFG,
                                                             GENERAL_TABLE_INDEX,
                                                             1,
                                                             CNFG_TO_INDEX,
                                                             1,
                                                             MAX_FIELD_NO,
                                                             0);
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_GET_CFG;
    param->payload = payloadString;
    processBar->loadProcessBar();
    applController->processActivity(currDevName, DEVICE_COMM, param);
}

void SearchCamera::slotCreateCMDRequest(SET_COMMAND_e cmdType,quint8 totalFeilds)
{
    sendCommand (cmdType,totalFeilds);
}

void SearchCamera::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    quint8 configCam = 0;
    quint8 maxPossibleCam = 0;

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
                {
                    DELETE_OBJ(cameraSearchProcess);
                    if((param->deviceStatus == CMD_SUCCESS) || (param->deviceStatus == CMD_REQUEST_IN_PROGRESS))
                    {
                        isUpdateBeforeSearch = true;
                        updateControlsforSearch (true);
                        if((searchRunningInfo == NULL) && (isCancelSend == false))
                        {
                            searchRunningInfo = new TextLabel((SCALE_WIDTH(550)),
                                                              SCALE_HEIGHT(30),
                                                              SCALE_FONT(18),
                                                              Multilang("Searching...") +  QString(" 0 ") +  Multilang("camera(s) found"),
                                                              this,
                                                              HIGHLITED_FONT_COLOR,
                                                              NORMAL_FONT_FAMILY,
                                                              ALIGN_CENTRE_X_START_Y);
                        }

                        getAcqListTimer->start ();
                    }
                    else
                    {
                        DELETE_OBJ(searchRunningInfo);
                        loadInfoPageMsg(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                        isUpdateBeforeSearch = false;
                        updateControlsforSearch (false);
                        updateSelCamState();
                    }
                }
                break;

                case CNCL_AUTO_SEARCH:
                {
                    DELETE_OBJ(cameraSearchProcess);
                    DELETE_OBJ(searchRunningInfo);
                    isUpdateBeforeSearch = false;
                    updateControlsforSearch (false);
                    updateSelCamState();
                    m_deletePage = true;
                }
                break;

                case GET_ACQ_LIST:
                {
                    if(isBlockListFlag == false)
                    {
                        updateList(param);
                    }

                    if(isCancelSend == false)
                    {
                        getAcqListTimer->start();
                    }
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
                }
                break;

                case ADD_CAMERAS:
                {
                    testCameraIndex = INVALID_CAMERA_INDEX;
                    if(param->deviceStatus != CMD_SUCCESS)
                    {
                        loadInfoPageMsg(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                        isBlockListFlag = false;
                        break;
                    }

                    qint16 tempCamIndex = -1;

                    for(qint16 indexIP = 0; indexIP < ipAddressList.length(); indexIP++)
                    {
                        if ((ipAddressList[indexIP].first != requestedIp) && (ipAddressList[indexIP].second != requestedIp))
                        {
                            continue;
                        }

                        tempCamIndex = indexIP;

                        if (onvifPortList.at(indexIP) == requestedPort)
                        {
                            break;
                        }
                    }

                    if(tempCamIndex >= 0)
                    {
                        payloadLib->parseDevCmdReply(true, param->payload);
                        quint8 tempCamNum = payloadLib->getCnfgArrayAtIndex(0).toUInt();

                        camNameList.replace (tempCamIndex,currentCameraName);
                        camNumList.replace (tempCamIndex,QString("%1").arg (tempCamNum));
                        camStatusList.replace (tempCamIndex,QString("%1").arg (1)); // MX_CAM_ADDED = 1

                        if((ipv4AddressStr[tempCamIndex - (MAX_CAM_SEARCH_RECORD_DATA*currentPageNo)]->getText () == requestedIp) ||
                                (ipv6AddressStr[tempCamIndex - (MAX_CAM_SEARCH_RECORD_DATA*currentPageNo)]->getText () == requestedIp))
                        {
                            addCameraBtn[tempCamIndex - (MAX_CAM_SEARCH_RECORD_DATA*currentPageNo)]->changeImageType (ADDED_BUTTON_TABLE_INDEX);
                            addCameraBtn[tempCamIndex - (MAX_CAM_SEARCH_RECORD_DATA*currentPageNo)]->update ();
                            testCameraBtn[tempCamIndex - (MAX_CAM_SEARCH_RECORD_DATA*currentPageNo)]->setIsEnabled (true);

                            if(onvifSupportStatusList.at (tempCamIndex).toUInt () == 0)
                            {
                                brandList.replace (tempCamIndex, currentBrandName);
                                modelList.replace (tempCamIndex, currentModelName);

                                brandsStr[tempCamIndex - (MAX_CAM_SEARCH_RECORD_DATA*currentPageNo)]->changeText (brandList.at (tempCamIndex));
                                brandsStr[tempCamIndex - (MAX_CAM_SEARCH_RECORD_DATA*currentPageNo)]->update ();
                                modelStr[tempCamIndex - (MAX_CAM_SEARCH_RECORD_DATA*currentPageNo)]->changeText (modelList.at (tempCamIndex));
                                modelStr[tempCamIndex - (MAX_CAM_SEARCH_RECORD_DATA*currentPageNo)]->update ();
                            }

                            selectCam[tempCamIndex - (MAX_CAM_SEARCH_RECORD_DATA*currentPageNo)]->setIsEnabled (false);
                            selectCam[tempCamIndex - (MAX_CAM_SEARCH_RECORD_DATA*currentPageNo)]->changeState (OFF_STATE);
                        }
                        else
                        {
                            showCameraSearchList();
                            updateSelCamState();
                        }

                        loadInfoPageMsg(ValidationMessage::getValidationMessage(CAM_SRCH_CAM_ADD_SUCCESS) + QString("%1").arg (tempCamNum));
                        testCameraIndex = payloadLib->getCnfgArrayAtIndex(0).toUInt();
                    }
                    isBlockListFlag = false;
                }
                break;

                case OTHR_SUP:
                {
                    if(testCam != NULL)
                    {
                        testCam->processDeviceResponse (param,deviceName);
                    }
                }
                break;

                case TST_CAM:
                {
                    if(param->deviceStatus == CMD_SUCCESS)
                    {
                        if(testCam == NULL)
                        {
                            autoAddIconTimer->stop();
                            if(testCameraIndex != INVALID_CAMERA_INDEX)
                            {
                                WizardCommon:: InfoPageImage();
                                testCam = new WizardTestCamera(camNumList.at (testCameraIndex).toUInt (),
                                                               camNameList.at (testCameraIndex),
                                                               payloadLib,
                                                               parentWidget ());
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
                    else
                    {
                        testCameraIndex = INVALID_CAMERA_INDEX;
                        loadInfoPageMsg (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                    }
                    processBar->unloadProcessBar();
                }
                break;

                case MAX_ADD_CAM:
                {
                    testCameraIndex = INVALID_CAMERA_INDEX;
                    if(param->deviceStatus == CMD_SUCCESS)
                    {
                        payloadLib->parseDevCmdReply(true, param->payload);
                        configCam = payloadLib->getCnfgArrayAtIndex(0).toUInt();
                        maxPossibleCam = (devTableInfo.ipCams - configCam);
                        if(maxPossibleCam == 0)
                        {
                            loadInfoPageMsg (ValidationMessage::getDeviceResponceMessage(CMD_MAX_CAM_CONFIGED));
                        }
                        else
                        {
                            if(autoAddIpList.length() <= maxPossibleCam)
                            {
                                sendAutoAddCmd();
                            }
                            else
                            {
                                loadInfoPageMsg (QString("%1").arg (configCam) +  " " + Multilang("camera(s) are already configured Only") + " "
                                                 + QString("%1").arg (devTableInfo.ipCams - configCam)
                                                 + " " + Multilang("camera(s) can be configured"));
                            }
                        }
                    }
                    else
                    {
                        loadInfoPageMsg(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                    }
                }
                break;

                case AUTO_CONFIGURE_CAMERA:
                {
                    processBar->unloadProcessBar();
                    autoAddIpList.clear();
                    if(param->deviceStatus == CMD_SUCCESS)
                    {
                        loadInfoPageMsg (ValidationMessage::getValidationMessage(CAM_SRCH_AUTO_CONFI));
                    }
                    else
                    {
                        testCameraIndex = INVALID_CAMERA_INDEX;

                        loadInfoPageMsg (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
                        isInfoPageLoaded = true;
                    }
                    updateSelCamState ();
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
                if (payloadLib->getcnfgTableIndex() == GENERAL_TABLE_INDEX)
                {
                    retainIpAddrInAutoConfig = payloadLib->getCnfgArrayAtIndex(FIELD_AUTO_CONFIG + FIELD_AUTO_CONFIG_IP_RETAIN + 1).toInt() ? true : false;
                    autoConfigIpAddrFamily = QHostAddress(payloadLib->getCnfgArrayAtIndex(FIELD_AUTO_CONFIG + FIELD_AUTO_CONFIG_START_IP + 1).toString()).protocol();
                }
            }

            if(addCamera != NULL)
            {
                addCamera->processDeviceResponse(param, deviceName);
            }
            m_getConfig = true;
        }
        break;

        case MSG_SET_CFG:
        {
            if(param->deviceStatus == CMD_SUCCESS)
            {
                MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
                processBar->loadProcessBar();
                sendAutoAddCmd();
            }
            else
            {
                InfoPageImage();
                infoPage->raise();
                infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
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

void SearchCamera::slotInfoPageBtnclick(int)
{
    WizardCommon::UnloadInfoPageImage();
    processBar->unloadProcessBar();
    loadInfoPageFlag = false;
    isInfoPageLoaded = false;
}

void SearchCamera::slotautoAddIconTimeOut()
{
    if((isInfoPageLoaded == false) && (addButton->getIsEnabled ()))
    {
        if(addButton->getCurrentImageType () == IMAGE_TYPE_MOUSE_HOVER)
        {
            addButton->changeImage (IMAGE_TYPE_NORMAL);
        }
        else
        {
            addButton->changeImage (IMAGE_TYPE_MOUSE_HOVER,true);
        }
    }
}
