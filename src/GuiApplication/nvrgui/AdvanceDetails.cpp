#include "AdvanceDetails.h"
#include <QPainter>
#include <QMouseEvent>
#include "ValidationMessage.h"

#define ADVANCE_DETAIL_WIDTH                SCALE_WIDTH(1126)
#define ADVANCE_DETAIL_HEIGHT               SCALE_HEIGHT(790)

#define TOP_RECT_WIDTH                      SCALE_WIDTH(305)
#define TOP_RECT_HEIGHT                     SCALE_HEIGHT(45)

#define INNER_RECT_WIDTH                    SCALE_WIDTH(1096)
#define INNER_RECT_HEIGHT                   SCALE_HEIGHT(565)

#define INTERFACE_TABLECELL_WIDTH           SCALE_WIDTH(167)
#define INTERFACE_TABLECELL_WIDTH_NW_PARAM  SCALE_WIDTH(345)
#define AUTO_REFRESH_INTERVAL_IN_MS         3000

const QString interfaceStr[] =
{
    "Interface",
    "Status",
    "IP Address",
    "Uplink (kbps)",
    "Downlink (kbps)",
    "LAN 1",
    "LAN 2",
    "Broadband"
};

const QString channelStr[] =
{
    "Camera",
    "Main (FPS/GOP)",
    "Sub (FPS/GOP)"
};

const QString lanstateStr[2] =
{
    "Up",
    "Down"
};

const QString modemstateStr[3] =
{
    "No Modem",
    "Not Connected",
    "Connected"
};

AdvanceDetails::AdvanceDetails(QWidget *parent)
    :BackGround((ApplController::getXPosOfScreen() + ((ApplController::getWidthOfScreen() - ADVANCE_DETAIL_WIDTH) / 2)),
                (ApplController::getYPosOfScreen() + ((ApplController::getHeightOfScreen() - ADVANCE_DETAIL_HEIGHT) / 2)),
                ADVANCE_DETAIL_WIDTH,
                (ADVANCE_DETAIL_HEIGHT - TOP_RECT_HEIGHT),
                BACKGROUND_TYPE_1,
                SYSTEM_STATUS_BUTTON,
                parent,
                TOP_RECT_WIDTH, TOP_RECT_HEIGHT,
                "Advance Status"),
    liveStream(0), pbStream(0), cpuTempValue(0)
{
    applController = ApplController::getInstance ();
    payloadLib = new PayloadLib();
    isCamField = true;
    responseStatus = false;
    nextPageSelected = false;
    currCamPage = 0;
    m_internetConn = false;
    cpuUseValue = 0;
    totalPages = 0;
    refreshInterval = NULL;

    for(quint8 index = 0; index < 2; index++)
    {
        daysParse[index] = 0;
        hourParse[index] = 0;
        minuteParse[index] = 0;
        secParse[index] = 0;
    }

    for(quint8 index = 0; index < MAX_CAMERAS; index++)
    {
        channelFpsValue[LIVE_STREAM_TYPE_MAIN][index] = channelFpsValue[LIVE_STREAM_TYPE_SUB][index] = 0;
        channelGopValue[LIVE_STREAM_TYPE_MAIN][index] = channelGopValue[LIVE_STREAM_TYPE_SUB][index] = 0;
    }

    createDefaultComponent ();

    cameraTab->setShowClickedImage (true);
    processBar = new ProcessBar(0, TOP_RECT_HEIGHT,
                                ADVANCE_DETAIL_WIDTH,
                                ADVANCE_DETAIL_HEIGHT - TOP_RECT_HEIGHT,
                                SCALE_WIDTH(15), this);

    infoPage = new InfoPage(0,
                            0,
                            ADVANCE_DETAIL_WIDTH,
                            ADVANCE_DETAIL_HEIGHT,
                            INFO_ADVANCE_DETAILS,
                            this);
    connect (infoPage,
             SIGNAL(sigInfoPageCnfgBtnClick(int)),
             this,
             SLOT(slotInfoPageCnfgBtnClick(int)));

    refreshInterval = new QTimer(this);
    connect(refreshInterval,
            SIGNAL(timeout()),
            this,
            SLOT(slotRefreshButtonClicked()));
    refreshInterval->setInterval(AUTO_REFRESH_INTERVAL_IN_MS);

    setDefaultFocus();
    this->show ();
}

AdvanceDetails::~AdvanceDetails ()
{
    delete payloadLib;
    delete processBar;

    if(IS_VALID_OBJ(refreshInterval))
    {
        if(refreshInterval->isActive ())
        {
            refreshInterval->stop ();
        }

        disconnect (refreshInterval,
                    SIGNAL(timeout()),
                    this,
                    SLOT(slotRefreshButtonClicked()));
        DELETE_OBJ(refreshInterval);
    }

    disconnect (infoPage,
                SIGNAL(sigInfoPageCnfgBtnClick(int)),
                this,
                SLOT(slotInfoPageCnfgBtnClick(int)));
    delete infoPage;

    disconnect (m_mainCloseButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpadateCurrentElement(int)));
    disconnect (deviceDropDown,
                SIGNAL(sigValueChanged(QString,quint32)),
                this,
                SLOT(slotSpinBoxValueChange(QString,quint32)));
    disconnect (deviceDropDown,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpadateCurrentElement(int)));
    disconnect(deviceDropDown,
               SIGNAL(sigDropDownListDestroyed()),
               this,
               SLOT(slotDropDownListDestroyed()));
    delete deviceDropDown;

    disconnect (cameraTab,
                SIGNAL(sigButtonClicked(int)),
                this,
                SLOT(slotTabSelected(int)));
    disconnect (cameraTab,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpadateCurrentElement(int)));
    delete cameraTab;

    disconnect (systemTab,
                SIGNAL(sigButtonClicked(int)),
                this,
                SLOT(slotTabSelected(int)));
    disconnect (systemTab,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpadateCurrentElement(int)));
    delete systemTab;

    delete innerRect;

    for(quint8 rIndex=0, bgIndex = 0, headerIndex = 0; bgIndex < MAX_BGTILES; bgIndex++)
    {
        for(quint8 row= 0; row < 3; row++)
        {
            delete channelHeader[headerIndex];
            delete channelHeaderStr[headerIndex++];
        }

        for(quint8 row = 0; row < MAX_CAMERA_IN_ONE_BG; row++)
        {
            quint8 hedIndx = (bgIndex*MAX_CAMERA_IN_ONE_BG  + row);
            delete channelHeaderCamNo[hedIndx];
            delete channelHeaderCamNoStr[hedIndx];
        }

        for(quint8 row = 0; row < 2; row++)
        {
            for(quint8 col = 0; col < MAX_CAMERA_IN_ONE_BG; col++)
            {
                delete channelField[rIndex][col];
                delete channelFieldStr[rIndex][col];
            }
            rIndex++;
        }
    }

    delete bgTile4;

    for(quint8 row = 0; row < MAX_ADV_DETAIL_INTERFACE_HEADER; row++)
    {
        delete interfaceHeader[row];
        delete interfaceHeaderStr[row];
    }

    for(quint8 row = 0; row < MAX_ADV_DETAIL_INTERFACE; row++)
    {
        for(quint8 col = 0; col < MAX_ADV_DETAIL_INTERFACE_FIELD; col++)
        {
            delete interfaceField[row][col];
        }

        delete networkStatusLabel[row];
        delete ipv4AddressLabel[row];
        delete ipv6AddressLabel[row];
        delete upLinkLabel[row];
        delete downLinkLabel[row];
    }

    delete bgTile5;
    delete networkStreamStr;
    delete livePbElement[0];
    delete livePbElement[1];

    delete bgTile6;
    delete systemStrHeader;

    delete systemFieldReadOnly[0];
    delete systemFieldReadOnly[1];
    delete systemFieldReadOnly[2];

    delete bgTile7;
    delete recRateStr;
    delete recRate[0];
    delete recRate[1];

    delete bgTile8;
    delete recStatusStr;
    delete recStatus[0];
    delete recStatus[1];
    delete recStatus[2];

    disconnect (backButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotBackButtonClicked(int)));
    disconnect (backButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpadateCurrentElement(int)));
    delete backButton;

    disconnect (prevButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotPrevNextCameraClicked(int)));
    disconnect (prevButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpadateCurrentElement(int)));
    delete prevButton;


    for(quint8 index = 0; index < MAX_ADV_STS_PAGE; index++)
    {
        disconnect(m_PageNumberLabel[index],
                   SIGNAL(sigMousePressClick(QString)),
                   this,
                   SLOT(slotPageNumberButtonClick(QString)));
        disconnect(m_PageNumberLabel[index],
                   SIGNAL(sigUpdateCurrentElement(int)),
                   this,
                   SLOT(slotUpadateCurrentElement(int)));
        delete m_PageNumberLabel[index];
    }

    disconnect (nextButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotPrevNextCameraClicked(int)));
    disconnect (nextButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpadateCurrentElement(int)));
    delete nextButton;
}

void AdvanceDetails::createDefaultComponent ()
{
    m_elementList[ADV_DETAIL_CLOSE_BUTTON] = m_mainCloseButton;
    m_mainCloseButton->setIndexInPage(ADV_DETAIL_CLOSE_BUTTON);
    connect (m_mainCloseButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpadateCurrentElement(int)));

    QMap<quint8, QString> deviceMapList;
    applController->GetDevNameDropdownMapList(deviceMapList);

    deviceDropDown = new DropDown(((ADVANCE_DETAIL_WIDTH - INNER_RECT_WIDTH) /2),
                                  SCALE_HEIGHT(60),
                                  BGTILE_MEDIUM_SIZE_WIDTH,
                                  BGTILE_HEIGHT,
                                  ADV_DETAIL_SPINBOX,
                                  DROPDOWNBOX_SIZE_200,
                                  "Devices",
                                  deviceMapList,
                                  this,
                                  "",
                                  false,
                                  0,
                                  NO_LAYER);
    m_elementList[ADV_DETAIL_SPINBOX] = deviceDropDown;
    connect (deviceDropDown,
             SIGNAL(sigValueChanged(QString,quint32)),
             this,
             SLOT(slotSpinBoxValueChange(QString,quint32)));
    connect (deviceDropDown,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpadateCurrentElement(int)));
    connect(deviceDropDown,
            SIGNAL(sigDropDownListDestroyed()),
            this,
            SLOT(slotDropDownListDestroyed()));

    cameraTab = new MenuButton(0,
                               SCALE_WIDTH(120),
                               SCALE_HEIGHT(30),
                               "Camera",
                               this,
                               0,
                               ((ADVANCE_DETAIL_WIDTH - INNER_RECT_WIDTH) /2),
                               (deviceDropDown->y () + deviceDropDown->height () + SCALE_HEIGHT(20)),
                               ADV_DETAIL_CAMERA_TAB);
    m_elementList[ADV_DETAIL_CAMERA_TAB] = cameraTab;
    connect (cameraTab,
             SIGNAL(sigButtonClicked(int)),
             this,
             SLOT(slotTabSelected(int)));
    connect (cameraTab,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpadateCurrentElement(int)));

    systemTab = new MenuButton(1,
                               SCALE_WIDTH(120),
                               SCALE_HEIGHT(30),
                               "System",
                               this,
                               0,
                               deviceDropDown->x () + SCALE_WIDTH(120),
                               SCALE_HEIGHT(90),
                               ADV_DETAIL_SYSTEM_TAB);
    m_elementList[ADV_DETAIL_SYSTEM_TAB] = systemTab;
    connect (systemTab,
             SIGNAL(sigButtonClicked(int)),
             this,
             SLOT(slotTabSelected(int)));
    connect (systemTab,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpadateCurrentElement(int)));

    innerRect = new Rectangle(((ADVANCE_DETAIL_WIDTH - INNER_RECT_WIDTH) /2),
                              cameraTab->y ()+ cameraTab->height (),
                              INNER_RECT_WIDTH,
                              INNER_RECT_HEIGHT, SCALE_WIDTH(5),
                              BORDER_2_COLOR,
                              CLICKED_BKG_COLOR,this, 2);

    quint8 rIndex = 0,headerIndex = 0;
    for(quint8 index = 0;index < MAX_BGTILES; index++)
    {
        camStateTiles[index] = new BgTile ((innerRect->x () + SCALE_WIDTH(30)),
                                           ((index == 0) ? (innerRect->y ()) : (camStateTiles[index-1]->y() + camStateTiles[index-1]->height())) + SCALE_HEIGHT(30),
                                           SCALE_WIDTH(140 + 7 + 7 + 1 + (MAX_CAMERA_IN_ONE_BG*55)),
                                           SCALE_HEIGHT(105),
                                           COMMON_LAYER,
                                           this);

        for(quint8 row = 0; row < 3; row++)
        {
            channelHeader[headerIndex] = new TableCell(camStateTiles[index]->x () + SCALE_WIDTH(7),
                                                       camStateTiles[index]->y () + SCALE_HEIGHT(7) + ((SCALE_HEIGHT(30)) * row),
                                                       SCALE_WIDTH(140), SCALE_HEIGHT(30), this, true);

            channelHeaderStr[headerIndex] = new TextLabel((channelHeader[headerIndex]->x () + (channelHeader[headerIndex]->width ()/2)),
                                                          (channelHeader[headerIndex]->y () + (channelHeader[headerIndex]->height ()/2)),
                                                          SCALE_WIDTH(SMALL_SUFFIX_FONT_SIZE), channelStr[row],
                                                          this, HIGHLITED_FONT_COLOR,
                                                          NORMAL_FONT_FAMILY,
                                                          ALIGN_CENTRE_X_CENTER_Y, 0, 0, SCALE_WIDTH(140));
            headerIndex++;
        }

        for(quint8 row= 0; row < MAX_CAMERA_IN_ONE_BG; row++)
        {
            quint8 hedIndx = (index*MAX_CAMERA_IN_ONE_BG  + row);
            channelHeaderCamNo[hedIndx] = new TableCell(camStateTiles[index]->x () + SCALE_WIDTH(7) + SCALE_WIDTH(140) + ((SCALE_WIDTH(55))*row),
                                                        camStateTiles[index]->y () + SCALE_HEIGHT(7),
                                                        SCALE_WIDTH(55), SCALE_HEIGHT(30), this, true);

            channelHeaderCamNoStr[hedIndx] = new TextLabel((channelHeaderCamNo[hedIndx]->x () + (channelHeaderCamNo[hedIndx]->width ()/2)),
                                                           (channelHeaderCamNo[hedIndx]->y () + (channelHeaderCamNo[hedIndx]->height ()/2)),
                                                           SCALE_WIDTH(SMALL_SUFFIX_FONT_SIZE),
                                                           QString("%1").arg (row + 1 + MAX_CAMERA_IN_ONE_BG*index),
                                                           this,
                                                           HIGHLITED_FONT_COLOR,
                                                           NORMAL_FONT_FAMILY,
                                                           ALIGN_CENTRE_X_CENTER_Y, 0, 0, SCALE_WIDTH(55));
        }

        for(quint8 row = 0; row < 2; row++)
        {
            for(quint8 col = 0; col < MAX_CAMERA_IN_ONE_BG; col++)
            {
                channelField[rIndex][col] = new TableCell((camStateTiles[index]->x () + SCALE_WIDTH(7) + SCALE_WIDTH(140) + ((SCALE_WIDTH(55)) * col)),
                                                          (camStateTiles[index]->y () + SCALE_HEIGHT(7) + SCALE_HEIGHT(30) + ((SCALE_HEIGHT(30)) * row)),
                                                          SCALE_WIDTH(55),
                                                          SCALE_HEIGHT(30),
                                                          this);

                channelFieldStr[rIndex][col] = new TextLabel(channelField[rIndex][col]->x () + channelField[rIndex][col]->width ()/2,
                                                             channelField[rIndex][col]->y () + channelField[rIndex][col]->height ()/2,
                                                             SCALE_WIDTH(SMALL_SUFFIX_FONT_SIZE),
                                                             "",
                                                             this,
                                                             NORMAL_FONT_COLOR,
                                                             NORMAL_FONT_FAMILY,
                                                             ALIGN_CENTRE_X_CENTER_Y, 0, 0, SCALE_WIDTH(55));
            }

            rIndex++;
        }
    }

    // Bgtile4 - LAN 1,2 Broadband field
    bgTile4 = new BgTile(innerRect->x() + SCALE_WIDTH(30),
                         innerRect->y () + SCALE_HEIGHT(8),
                         SCALE_WIDTH(1036), SCALE_HEIGHT(230),
                         COMMON_LAYER, this);

    bgTile4->setVisible (false);

    /* Creating table cell for titles : Interface, Status, IP Address, Uplink (kbps), Downink (kbps) */
    for(quint8 row = 0; row < ADV_DETAIL_LAN1; row++)
    {
        if (row == ADV_DETAIL_IP_ADDR)
        {
            interfaceHeader[row] = new TableCell((bgTile4->x () + SCALE_WIDTH(10) + (INTERFACE_TABLECELL_WIDTH * row)),
                                                 (bgTile4->y () + SCALE_HEIGHT(10)),
                                                 INTERFACE_TABLECELL_WIDTH_NW_PARAM, SCALE_HEIGHT(30), this, true);
        }
        else if (row == ADV_DETAIL_UPLINK || row == ADV_DETAIL_DOWNLINK)
        {
            interfaceHeader[row] = new TableCell((bgTile4->x () + SCALE_WIDTH(10) + (INTERFACE_TABLECELL_WIDTH * (row -1) + INTERFACE_TABLECELL_WIDTH_NW_PARAM)),
                                                 (bgTile4->y () + SCALE_HEIGHT(10)),
                                                 INTERFACE_TABLECELL_WIDTH, SCALE_HEIGHT(30), this, true);
        }
        else
        {
            interfaceHeader[row] = new TableCell((bgTile4->x () + SCALE_WIDTH(10) + (INTERFACE_TABLECELL_WIDTH * row)),
                                                 (bgTile4->y () + SCALE_HEIGHT(10)),
                                                 INTERFACE_TABLECELL_WIDTH, SCALE_HEIGHT(30), this, true);
        }
    }

    /* Creating table cell for titles : LAN 1, LAN 2, Broadband */
    interfaceHeader[ADV_DETAIL_LAN1] = new TableCell((bgTile4->x () + SCALE_WIDTH(10)),
                                       (bgTile4->y () + SCALE_HEIGHT(10) + SCALE_HEIGHT(30)),
                                       INTERFACE_TABLECELL_WIDTH, SCALE_HEIGHT(60), this, true);

    interfaceHeader[ADV_DETAIL_LAN2] = new TableCell((bgTile4->x () + SCALE_WIDTH(10)),
                                       (bgTile4->y () + SCALE_HEIGHT(10) + SCALE_HEIGHT(90)),
                                       INTERFACE_TABLECELL_WIDTH, SCALE_HEIGHT(60), this, true);

    interfaceHeader[ADV_DETAIL_BROADBAND] = new TableCell((bgTile4->x () + SCALE_WIDTH(10)),
                                       (bgTile4->y () + SCALE_HEIGHT(10) + SCALE_HEIGHT(150)),
                                       INTERFACE_TABLECELL_WIDTH, SCALE_HEIGHT(60), this, true);

    /* Creating text labels :
     * Row    - Interface, Status, IP Address, Uplink (kbps), Downink (kbps)
     * Column - LAN 1, LAN 2, Broadband */
    for(quint8 row = 0; row < MAX_ADV_DETAIL_INTERFACE_HEADER; row++)
    {
        interfaceHeaderStr[row] = new TextLabel((interfaceHeader[row]->x () + SCALE_WIDTH(10)),
                                                (interfaceHeader[row]->y () + ((interfaceHeader[row]->height ())/2)),
                                                NORMAL_FONT_SIZE, interfaceStr[row],
                                                this, HIGHLITED_FONT_COLOR,
                                                NORMAL_FONT_FAMILY,
                                                ALIGN_START_X_CENTRE_Y, 0, 0, INTERFACE_TABLECELL_WIDTH);
        interfaceHeader[row]->setVisible (false);
        interfaceHeaderStr[row]->setVisible (false);
    }

    /* Creating table cells to show field values for LAN 1, LAN 2 and Broadband */
    for(quint8 row = 0; row < MAX_ADV_DETAIL_INTERFACE; row++)
    {
        for(quint8 col = 0; col < MAX_ADV_DETAIL_INTERFACE_FIELD; col++)
        {
            if(col == ADV_DETAIL_IP_ADDR_FILED)
            {
                interfaceField[row][col] = new TableCell((bgTile4->x () + SCALE_WIDTH(10) + INTERFACE_TABLECELL_WIDTH + (INTERFACE_TABLECELL_WIDTH * col)),
                                                         (bgTile4->y () + SCALE_HEIGHT(10) + SCALE_HEIGHT(30) + ((SCALE_HEIGHT(60)) * row)),
                                                         INTERFACE_TABLECELL_WIDTH_NW_PARAM,
                                                         SCALE_HEIGHT(60),
                                                         this);
            }
            else if(col == ADV_DETAIL_UPLINK_FIELD || col == ADV_DETAIL_DOWNLINK_FIELD)
            {
                interfaceField[row][col] = new TableCell((bgTile4->x () + SCALE_WIDTH(10) + INTERFACE_TABLECELL_WIDTH_NW_PARAM + (INTERFACE_TABLECELL_WIDTH * col)),
                                                     	(bgTile4->y () + SCALE_HEIGHT(10) + SCALE_HEIGHT(30) + ((SCALE_HEIGHT(60)) * row)),
                                                     	INTERFACE_TABLECELL_WIDTH,
                                                     	SCALE_HEIGHT(60),
                                                     	this);
            }
            else
            {
                interfaceField[row][col] = new TableCell((bgTile4->x () + SCALE_WIDTH(10) + INTERFACE_TABLECELL_WIDTH + (INTERFACE_TABLECELL_WIDTH * col)),
                                                     	(bgTile4->y () + SCALE_HEIGHT(10) + SCALE_HEIGHT(30) + ((SCALE_HEIGHT(60)) * row)),
                                                     	INTERFACE_TABLECELL_WIDTH,
                                                     	SCALE_HEIGHT(60),
                                                     	this);
            }

            interfaceField[row][col]->setVisible (false);
        }
    }

    /* Creating text labels showig field values for LAN 1, LAN 2 and Broadband */
    for(quint8 row = 0; row < MAX_ADV_DETAIL_INTERFACE; row++)
    {
        networkStatusLabel[row] = new TextLabel(bgTile4->x () + SCALE_WIDTH(20) + (INTERFACE_TABLECELL_WIDTH),
                                               bgTile4->y () + SCALE_HEIGHT(38) + (SCALE_HEIGHT(60) * row) + SCALE_HEIGHT(30),
                                               NORMAL_FONT_SIZE, "", this,
                                               NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY,
                                               ALIGN_START_X_CENTRE_Y, 0, 0, INTERFACE_TABLECELL_WIDTH - SCALE_WIDTH(15));
        networkStatusLabel[row]->setVisible (false);

        ipv4AddressLabel[row] = new TextLabel(bgTile4->x () + SCALE_WIDTH(20) + (INTERFACE_TABLECELL_WIDTH * 2),
                                             bgTile4->y () + SCALE_HEIGHT(38) + (SCALE_HEIGHT(60) * row) + SCALE_HEIGHT(15),
                                             NORMAL_FONT_SIZE, "", this,
                                             NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY,
                                             ALIGN_START_X_CENTRE_Y, 0, 0, INTERFACE_TABLECELL_WIDTH - SCALE_WIDTH(15));
        ipv4AddressLabel[row]->setVisible (false);

        ipv6AddressLabel[row] = new TextLabel(bgTile4->x () + SCALE_WIDTH(20) + (INTERFACE_TABLECELL_WIDTH * 2),
                                             bgTile4->y () + SCALE_HEIGHT(38) + (SCALE_HEIGHT(60) * row) + SCALE_HEIGHT(45),
                                             NORMAL_FONT_SIZE, "", this,
                                             NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY,
                                             ALIGN_START_X_CENTRE_Y, 0, 0, INTERFACE_TABLECELL_WIDTH_NW_PARAM - SCALE_WIDTH(15));
        ipv6AddressLabel[row]->setVisible (false);

        upLinkLabel[row] = new TextLabel(bgTile4->x () + SCALE_WIDTH(20) + INTERFACE_TABLECELL_WIDTH_NW_PARAM + (INTERFACE_TABLECELL_WIDTH * 2),
                                        bgTile4->y () + SCALE_HEIGHT(38) + (SCALE_HEIGHT(60) * row) + SCALE_HEIGHT(30),
                                        NORMAL_FONT_SIZE, "", this,
                                        NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY,
                                        ALIGN_START_X_CENTRE_Y, 0, 0, INTERFACE_TABLECELL_WIDTH - SCALE_WIDTH(15));

        upLinkLabel[row]->setVisible (false);

        downLinkLabel[row] = new TextLabel(bgTile4->x () + SCALE_WIDTH(20) + INTERFACE_TABLECELL_WIDTH_NW_PARAM + (INTERFACE_TABLECELL_WIDTH * 3),
                                          bgTile4->y () + SCALE_HEIGHT(38) + (SCALE_HEIGHT(60) * row) + SCALE_HEIGHT(30),
                                          NORMAL_FONT_SIZE, "", this,
                                          NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY,
                                          ALIGN_START_X_CENTRE_Y, 0, 0, INTERFACE_TABLECELL_WIDTH - SCALE_WIDTH(15));

        downLinkLabel[row]->setVisible (false);
    }

    // Bgtile4 - Network stream
    bgTile5 = new BgTile(bgTile4->x(),
                         bgTile4->y ()+ bgTile4->height () + SCALE_HEIGHT(5),
                         SCALE_WIDTH(515), SCALE_HEIGHT(133),
                         COMMON_LAYER,
                         this);
    bgTile5->setVisible (false);

    networkStreamStr = new TextLabel(bgTile5->x ()+ SCALE_WIDTH(15),
                                     bgTile5->y ()+ SCALE_HEIGHT(10),
                                     NORMAL_FONT_SIZE, "Network Streams",
                                     this, HIGHLITED_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                     0, 0, SCALE_WIDTH(515));
    networkStreamStr->setVisible (false);

    livePbElement[0] = new ReadOnlyElement((bgTile5->x () + SCALE_WIDTH(10)),
                                           (bgTile5->y () + SCALE_HEIGHT(40)),
                                           SCALE_WIDTH(495), SCALE_HEIGHT(40), SCALE_WIDTH(240), SCALE_HEIGHT(30), "",this,
                                           COMMON_LAYER, -1, SCALE_WIDTH(10),
                                           "Live");
    livePbElement[0]->setVisible (false);

    livePbElement[1] = new ReadOnlyElement((bgTile5->x () + SCALE_WIDTH(10)),
                                           (bgTile5->y () + SCALE_HEIGHT(80)),
                                           SCALE_WIDTH(495), SCALE_HEIGHT(40), SCALE_WIDTH(240) , SCALE_HEIGHT(30) , "",this,
                                           COMMON_LAYER, -1, SCALE_WIDTH(10), "Playback");
    livePbElement[1]->setVisible (false);

    // Bgtile4 - Recording rate
    bgTile6 = new BgTile(bgTile5->x ()+ bgTile5->width () + SCALE_WIDTH(5),
                         bgTile5->y (),
                         SCALE_WIDTH(515), SCALE_HEIGHT(133),
                         COMMON_LAYER,
                         this);
    bgTile6->setVisible (false);

    recRateStr = new TextLabel(bgTile6->x ()+SCALE_WIDTH(15),
                               bgTile6->y ()+SCALE_HEIGHT(10),
                               NORMAL_FONT_SIZE, "Current Recording Rate",
                               this, HIGHLITED_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                               0, 0, SCALE_WIDTH(515));
    recRateStr->setVisible (false);

    recRate[0] = new ReadOnlyElement((bgTile6->x () + SCALE_WIDTH(10)),
                                     (bgTile6->y () + SCALE_HEIGHT(40)),
                                     SCALE_WIDTH(495), SCALE_HEIGHT(40), SCALE_WIDTH(240), SCALE_HEIGHT(30), "",this,
                                     COMMON_LAYER, -1, SCALE_WIDTH(10),
                                     "Per Hour", "MB", NORMAL_FONT_COLOR,
                                     NORMAL_FONT_SIZE);
    recRate[0]->setVisible (false);

    recRate[1] = new ReadOnlyElement((bgTile6->x () + SCALE_WIDTH(10)),
                                     (bgTile6->y () + SCALE_HEIGHT(80)),
                                     SCALE_WIDTH(495), SCALE_HEIGHT(40), SCALE_WIDTH(240) , SCALE_HEIGHT(30) , "",this,
                                     COMMON_LAYER, -1, SCALE_WIDTH(10), "Per Day", "MB",
                                     NORMAL_FONT_COLOR, NORMAL_FONT_SIZE);
    recRate[1]->setVisible (false);

    // Bgtile7 - cpu status
    bgTile7 = new BgTile(bgTile5->x (),
                         bgTile5->y () + bgTile5->height () + SCALE_HEIGHT(5),
                         SCALE_WIDTH(515), SCALE_HEIGHT(175),
                         COMMON_LAYER,
                         this);
    bgTile7->setVisible (false);

    systemStrHeader = new TextLabel(bgTile7->x ()+SCALE_WIDTH(15),
                                    bgTile7->y ()+SCALE_HEIGHT(10),
                                    NORMAL_FONT_SIZE, "CPU Status",
                                    this, HIGHLITED_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                    0, 0, SCALE_WIDTH(515));
    systemStrHeader->setVisible (false);

    systemFieldReadOnly[0] = new ReadOnlyElement((bgTile7->x () + SCALE_WIDTH(10)),
                                                 (bgTile7->y () + SCALE_HEIGHT(40)),
                                                 SCALE_WIDTH(495), SCALE_HEIGHT(40), SCALE_WIDTH(240), SCALE_HEIGHT(30), "",this,
                                                 COMMON_LAYER,-1 , SCALE_WIDTH(10),"Usage");
    systemFieldReadOnly[0]->setVisible (false);

    systemFieldReadOnly[1] = new ReadOnlyElement((bgTile7->x () + SCALE_WIDTH(10)),
                                                 (bgTile7->y () + SCALE_HEIGHT(80)),
                                                 SCALE_WIDTH(495), SCALE_HEIGHT(40), SCALE_WIDTH(240), SCALE_HEIGHT(30) , "",this,
                                                 COMMON_LAYER, -1, SCALE_WIDTH(10), "Up Time");
    systemFieldReadOnly[1]->setVisible (false);

    systemFieldReadOnly[2] = new ReadOnlyElement((bgTile7->x () + SCALE_WIDTH(10)),
                                                (bgTile7->y () + SCALE_HEIGHT(120)),
                                                SCALE_WIDTH(495), SCALE_HEIGHT(40), SCALE_WIDTH(240), SCALE_HEIGHT(30), "",this,
                                                COMMON_LAYER,-1 , SCALE_WIDTH(10),"Temperature");
    systemFieldReadOnly[2]->setVisible (false);

    // Bgtile8 - current status bgtile
    bgTile8 = new BgTile(bgTile7->x ()+ bgTile7->width () + SCALE_WIDTH(5),
                         bgTile7->y (),
                         SCALE_WIDTH(515), SCALE_HEIGHT(175),
                         COMMON_LAYER,
                         this);
    bgTile8->setVisible (false);

    recStatusStr = new TextLabel(bgTile8->x ()+SCALE_WIDTH(15),
                                 bgTile8->y ()+SCALE_HEIGHT(10),
                                 NORMAL_FONT_SIZE, "Current Status",
                                 this, HIGHLITED_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                 0, 0, SCALE_WIDTH(515));
    recStatusStr->setVisible (false);

    recStatus[0] = new ReadOnlyElement((bgTile8->x () + SCALE_WIDTH(10)),
                                       (bgTile8->y () + SCALE_HEIGHT(40)),
                                       SCALE_WIDTH(495), SCALE_HEIGHT(40), SCALE_WIDTH(240), SCALE_HEIGHT(30), "",this,
                                       COMMON_LAYER, -1, SCALE_WIDTH(10),
                                       "Active Recording Streams");
    recStatus[0]->setVisible (false);

    recStatus[1] = new ReadOnlyElement((bgTile8->x () + SCALE_WIDTH(10)),
                                       (bgTile8->y () + SCALE_HEIGHT(80)),
                                       SCALE_WIDTH(495), SCALE_HEIGHT(40), SCALE_WIDTH(240), SCALE_HEIGHT(30) , "",this,
                                       COMMON_LAYER, -1, SCALE_WIDTH(10), "Estimated Storage");
    recStatus[1]->setVisible (false);

    recStatus[2] = new ReadOnlyElement((bgTile8->x () + SCALE_WIDTH(10)),
                                       (bgTile8->y () + SCALE_HEIGHT(120)),
                                       SCALE_WIDTH(495), SCALE_HEIGHT(40), SCALE_WIDTH(240), SCALE_HEIGHT(30) , "",this,
                                       COMMON_LAYER, -1, SCALE_WIDTH(10), "Internet");
    recStatus[2]->setVisible (false);

    for(quint8 index = 0; index < MAX_ADV_STS_PAGE; index++)
    {
        m_PageNumberLabel[index] = new TextWithBackground((camStateTiles[MAX_BGTILES-1]->x() + (SCALE_WIDTH(490) + (index*(SCALE_WIDTH(40))))),
                                                          camStateTiles[MAX_BGTILES-1]->y() + camStateTiles[MAX_BGTILES-1]->height() + SCALE_HEIGHT(40),
                                                          NORMAL_FONT_SIZE,
                                                          "",
                                                          this,
                                                          NORMAL_FONT_COLOR,
                                                          NORMAL_FONT_FAMILY,
                                                          ALIGN_START_X_START_Y,
                                                          0,
                                                          false,
                                                          TRANSPARENTKEY_COLOR,
                                                          true,
                                                          (ADV_DETAIL_PAGE_NUM_BTN + index));
        m_elementList[(ADV_DETAIL_PAGE_NUM_BTN + index)] = m_PageNumberLabel[index];
        connect(m_PageNumberLabel[index],
                SIGNAL(sigMousePressClick(QString)),
                this,
                SLOT(slotPageNumberButtonClick(QString)));

        connect(m_PageNumberLabel[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpadateCurrentElement(int)));
    }

    currCamPage = 0;
    for(quint8 index = 0; index < MAX_ADV_STS_PAGE; index++)
    {
        m_PageNumberLabel[index]->changeText(QString(" ") + QString("%1").arg(currCamPage + 1 + index) + QString(" "));
        if((index + currCamPage) == currCamPage)
        {
            m_PageNumberLabel[index]->setBackGroundColor (CLICKED_BKG_COLOR);
            m_PageNumberLabel[index]->changeTextColor (HIGHLITED_FONT_COLOR);
            m_PageNumberLabel[index]->setBold (true);
            m_PageNumberLabel[index]->forceActiveFocus ();
        }
        else
        {
            m_PageNumberLabel[index]->setBackGroundColor (TRANSPARENTKEY_COLOR);
            m_PageNumberLabel[index]->changeTextColor (NORMAL_FONT_COLOR);
            m_PageNumberLabel[index]->setBold (false);
            m_PageNumberLabel[index]->deSelectControl ();
        }

        m_PageNumberLabel[index]->update();
    }

    prevButton = new ControlButton(PREVIOUS_BUTTON_INDEX, m_PageNumberLabel[0]->x() - SCALE_WIDTH(60),
                                   m_PageNumberLabel[0]->y() - SCALE_HEIGHT(7),
                                   BGTILE_SMALL_SIZE_WIDTH,
                                   BGTILE_HEIGHT,
                                   this,
                                   NO_LAYER, -1, "", true,
                                   ADV_DETAIL_PREV_BTN, false);
    m_elementList[ADV_DETAIL_PREV_BTN] = prevButton;
    connect (prevButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotPrevNextCameraClicked(int)));
    connect (prevButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpadateCurrentElement(int)));

    nextButton = new ControlButton(NEXT_BUTTON_INDEX, (m_PageNumberLabel[MAX_ADV_STS_PAGE - 1]->x() + SCALE_WIDTH(60)),
                                   m_PageNumberLabel[MAX_ADV_STS_PAGE - 1]->y() - SCALE_HEIGHT(7),
                                   BGTILE_SMALL_SIZE_WIDTH,
                                   BGTILE_HEIGHT,
                                   this,
                                   NO_LAYER, -1, "", true,
                                   ADV_DETAIL_NEXT_BTN);
    m_elementList[ADV_DETAIL_NEXT_BTN] = nextButton;
    connect (nextButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotPrevNextCameraClicked(int)));
    connect (nextButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpadateCurrentElement(int)));

    backButton = new PageOpenButton(ADVANCE_DETAIL_WIDTH - SCALE_WIDTH(100),
                                    ADVANCE_DETAIL_HEIGHT - SCALE_HEIGHT(55),
                                    BGTILE_MEDIUM_SIZE_WIDTH,
                                    SCALE_HEIGHT(40),
                                    ADV_DETAIL_PREV_PAGE_BUTTON,
                                    PAGEOPENBUTTON_MEDIAM_BACK,
                                    "Back",
                                    this,"", "",
                                    false, 0, NO_LAYER);
    m_elementList[ADV_DETAIL_PREV_PAGE_BUTTON] = backButton;
    connect (backButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotBackButtonClicked(int)));
    connect (backButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpadateCurrentElement(int)));
}

bool AdvanceDetails::getAdvanceDetailFrmDev(QString devName, SET_COMMAND_e command)
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = command;

    if (command == ADVANCE_STS_CAMERA)
    {
        quint8 fieldIdx = 0;

        /* Older device may not provide the information against this demand. Handle with care. */
        payloadLib->setCnfgArrayAtIndex(fieldIdx++, TRUE);  /* need camera sub-stream information */
        payloadLib->setCnfgArrayAtIndex(fieldIdx++, TRUE);  /* need 96 camera info (including camera 65 to 96)*/
        param->payload = payloadLib->createDevCmdPayload(fieldIdx);
    }

    if(applController->processActivity(devName, DEVICE_COMM, param) == false)
    {
        clearDetails();
        processBar->unloadProcessBar();
        infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(CMD_DEV_DISCONNECTED));
    }
    return true;
}

void AdvanceDetails::convertSecToHMS(int sec, quint8 index)
{
    secParse[index] = (sec % 60);
    sec = sec / 60;
    minuteParse[index] = (sec % 60);
    sec = sec / 60;
    hourParse[index] = (sec % 24);
    sec = sec / 24;
    daysParse[index] = sec;
}

void AdvanceDetails::resetVisibleElements()
{
    if(isCamField)
    {
        showHideSystemTiles (false);

        if(devTable.totalCams <= (MAX_BGTILES*MAX_CAMERA_IN_ONE_BG))
        {
            prevButton->setVisible (false);
            prevButton->setIsEnabled (false);
            nextButton->setVisible (false);
            nextButton->setIsEnabled (false);

            for(quint8 index = 0; index < MAX_ADV_STS_PAGE; index++)
            {
                m_PageNumberLabel[index]->setVisible (false);
                m_PageNumberLabel[index]->setIsEnabled (false);
            }
        }
        else
        {
            prevButton->setVisible (true);
            nextButton->setVisible (true);

            if(currCamPage != 0)
            {
                prevButton->setIsEnabled (true);
                m_elementList[m_currElement]->forceActiveFocus ();
            }
            else
            {
                if((m_currElement == ADV_DETAIL_NEXT_BTN) || (m_currElement == ADV_DETAIL_PREV_BTN))
                {
                    m_currElement = ADV_DETAIL_NEXT_BTN;
                    m_elementList[m_currElement]->forceActiveFocus ();
                }
                prevButton->setIsEnabled (false);
            }

            if(currCamPage == (totalPages - 1))
            {
                if((m_currElement == ADV_DETAIL_NEXT_BTN) || (m_currElement == ADV_DETAIL_PREV_BTN))
                {
                    m_currElement = ADV_DETAIL_PREV_BTN;
                    m_elementList[m_currElement]->forceActiveFocus ();
                }
                nextButton->setIsEnabled (false);
            }
            else
            {
                nextButton->setIsEnabled (true);
                m_elementList[m_currElement]->forceActiveFocus ();
            }

            quint8 maxVisblePageNumVisible = (totalPages < MAX_ADV_STS_PAGE) ? totalPages : MAX_ADV_STS_PAGE;

            for(quint8 index = 0; index < maxVisblePageNumVisible; index++)
            {
                m_PageNumberLabel[index]->setVisible (true);
                m_PageNumberLabel[index]->setIsEnabled (true);
            }

            for(quint8 index = maxVisblePageNumVisible; index < MAX_ADV_STS_PAGE; index++)
            {
                m_PageNumberLabel[index]->setVisible (false);
                m_PageNumberLabel[index]->setIsEnabled (false);
            }

            for(quint8 index = 0; index < MAX_ADV_STS_PAGE; index++)
            {
                if((currCamPage % MAX_ADV_STS_PAGE) == index)
                {
                    m_PageNumberLabel[index]->setBackGroundColor (CLICKED_BKG_COLOR);
                    m_PageNumberLabel[index]->changeTextColor (HIGHLITED_FONT_COLOR);
                    m_PageNumberLabel[index]->setBold (true);
                    m_PageNumberLabel[index]->forceActiveFocus ();
                    m_currElement = (ADV_DETAIL_PAGE_NUM_BTN + index);
                }
                else
                {
                    m_PageNumberLabel[index]->setBackGroundColor (TRANSPARENTKEY_COLOR);
                    m_PageNumberLabel[index]->changeTextColor (NORMAL_FONT_COLOR);
                    m_PageNumberLabel[index]->setBold (false);
                    m_PageNumberLabel[index]->deSelectControl ();
                }

                m_PageNumberLabel[index]->update();
            }
        }
        quint8 maxCamTileInCurPage = (MAX_BGTILES * MAX_CAMERA_IN_ONE_BG * currCamPage);
        if(devTable.totalCams  <= (MAX_CAMERA_IN_ONE_BG + maxCamTileInCurPage))
        {
            showHideCamTile (0, true,(devTable.totalCams - maxCamTileInCurPage));
            showHideCamTile (1, false);
            showHideCamTile (2, false);
            showHideCamTile (3, false);
        }
        else if (devTable.totalCams <= ((2 * MAX_CAMERA_IN_ONE_BG) + maxCamTileInCurPage))
        {
            showHideCamTile (0, true);
            showHideCamTile (1, true,(devTable.totalCams - MAX_CAMERA_IN_ONE_BG - maxCamTileInCurPage));
            showHideCamTile (2, false);
            showHideCamTile (3, false);
        }
        else if (devTable.totalCams <= ((3 * MAX_CAMERA_IN_ONE_BG) + maxCamTileInCurPage))
        {
            showHideCamTile (0, true);
            showHideCamTile (1, true);
            showHideCamTile (2, true,(devTable.totalCams - (2 * MAX_CAMERA_IN_ONE_BG) - maxCamTileInCurPage));
            showHideCamTile (3, false);
        }
        else
        {
            showHideCamTile (0, true);
            showHideCamTile (1, true);
            showHideCamTile (2, true);
            showHideCamTile (3, true);
        }
    }
    else
    {
        showHideCamTile (0, false);
        showHideCamTile (1, false);
        showHideCamTile (2, false);
        showHideCamTile (3, false);

        prevButton->setIsEnabled (false);
        prevButton->setVisible (false);
        nextButton->setIsEnabled (false);
        nextButton->setVisible (false);

        for(quint8 index = 0; index < MAX_ADV_STS_PAGE; index++)
        {
            m_PageNumberLabel[index]->setVisible (false);
            m_PageNumberLabel[index]->setIsEnabled (false);
        }

        showHideSystemTiles (true);
    }

    if(!responseStatus)
    {
        clearDetails ();
    }
}

void AdvanceDetails::showHideCamTile(quint8 tileIndex, bool showTile, quint8 totalCamInRow)
{
    if(showTile == false)
    {
        totalCamInRow = 0;
    }

    quint8 rIndex = (tileIndex*2);
    quint8 headerIndex = (tileIndex*3);

    camStateTiles[tileIndex]->setVisible (showTile);

    for(quint8 row = 0; row < 3; row++)
    {
        channelHeader[headerIndex]->setVisible (showTile);
        channelHeaderStr[headerIndex]->setVisible (showTile);
        headerIndex++;
    }

    for(quint8 col = 0; col < totalCamInRow; col++)
    {
        channelHeaderCamNo[(tileIndex*MAX_CAMERA_IN_ONE_BG + col)]->setVisible(showTile);
        channelHeaderCamNoStr[(tileIndex*MAX_CAMERA_IN_ONE_BG + col)]->setVisible(showTile);
        channelHeaderCamNoStr[(tileIndex*MAX_CAMERA_IN_ONE_BG + col)]->changeText
                (QString("%1").arg(col + 1 + (MAX_CAMERA_IN_ONE_BG * tileIndex) + (CAMERA_BIT_WISE_MAX * currCamPage)));
        channelHeaderCamNoStr[(tileIndex*MAX_CAMERA_IN_ONE_BG  + col)]->update();
    }

    for(quint8 row = totalCamInRow; row < MAX_CAMERA_IN_ONE_BG; row++)
    {
        channelHeaderCamNo[(tileIndex*MAX_CAMERA_IN_ONE_BG  + row)]->setVisible (false);
        channelHeaderCamNoStr[(tileIndex*MAX_CAMERA_IN_ONE_BG  + row)]->setVisible (false);
    }

    for(quint8 row = 0; row < MAX_LIVE_STREAM_TYPE; row++)
    {
        for(quint8 col = 0; col < totalCamInRow; col++)
        {
            quint8 val = col + (MAX_CAMERA_IN_ONE_BG * tileIndex) + (MAX_BGTILES * MAX_CAMERA_IN_ONE_BG * currCamPage);
            channelField[rIndex][col]->setVisible (showTile);
            channelFieldStr[rIndex][col]->changeText (QString("%1%2%3").arg(channelFpsValue[row][val]).arg("/").arg(channelGopValue[row][val]));
            channelFieldStr[rIndex][col]->setVisible (showTile);
            channelFieldStr[rIndex][col]->update();
        }

        for(quint8 col = totalCamInRow; col < MAX_CAMERA_IN_ONE_BG; col++)
        {
            channelField[rIndex][col]->setVisible (false);
            channelFieldStr[rIndex][col]->setVisible (false);
        }
        rIndex++;
    }

    camStateTiles[tileIndex]->resetGeometry (camStateTiles[tileIndex]->x (),
                                             camStateTiles[tileIndex]->y (),
                                             SCALE_WIDTH(140 + 7 + 7 + 1) + (totalCamInRow*(SCALE_WIDTH(55))),
                                             camStateTiles[tileIndex]->height ());
}

void AdvanceDetails:: showHideSystemTiles(bool showTile)
{
    if(showTile)
    {
        bgTile4->setVisible (true);

        if(devTable.numOfLan == 1)
        {
            bgTile4->resetGeometry (SCALE_WIDTH(1036), (SCALE_HEIGHT(230) - SCALE_HEIGHT(60)));
            for(quint8 row = 0; row < ADV_DETAIL_LAN2; row++)
            {
                interfaceHeader[row]->setVisible (true);
                interfaceHeaderStr[row]->setVisible (true);
            }
            interfaceHeaderStr[ADV_DETAIL_LAN2]->changeText (interfaceStr[7]);
            interfaceHeader[ADV_DETAIL_LAN2]->setVisible (true);
            interfaceHeaderStr[ADV_DETAIL_LAN2]->setVisible (true);

            interfaceHeader[ADV_DETAIL_BROADBAND]->setVisible (false);
            interfaceHeaderStr[ADV_DETAIL_BROADBAND]->setVisible (false);

            for(quint8 col = 0; col < MAX_ADV_DETAIL_INTERFACE_FIELD; col++)
            {
                interfaceField[ADV_DETAIL_INTERFACE_LAN1][col]->setVisible (true);
                interfaceField[ADV_DETAIL_INTERFACE_LAN2][col]->setVisible (true);
                interfaceField[ADV_DETAIL_INTERFACE_BROADBAND][col]->setVisible (false);
            }

            for (quint8 row = 0; row < MAX_ADV_DETAIL_INTERFACE; row++)
            {
                if(row == ADV_DETAIL_INTERFACE_BROADBAND)
                {
                    networkStatusLabel[row]->setVisible (false);
                    ipv4AddressLabel[row]->setVisible (false);
                    ipv6AddressLabel[row]->setVisible (false);
                    upLinkLabel[row]->setVisible (false);
                    downLinkLabel[row]->setVisible (false);
                }
                else
                {
                    quint8 ifaceDataRow = (row == ADV_DETAIL_INTERFACE_LAN2) ? (quint8)ADV_DETAIL_INTERFACE_BROADBAND : row;
                    networkStatusLabel[row]->changeText(networkStatusStr[ifaceDataRow]);
                    networkStatusLabel[row]->setVisible (true);
                    ipv4AddressLabel[row]->changeText(ipv4AddrStr[ifaceDataRow]);
                    ipv4AddressLabel[row]->setVisible (true);
                    ipv6AddressLabel[row]->changeText(ipv6AddrStr[ifaceDataRow]);
                    ipv6AddressLabel[row]->setVisible (true);
                    upLinkLabel[row]->changeText(upLinkStr[ifaceDataRow]);
                    upLinkLabel[row]->setVisible (true);
                    downLinkLabel[row]->changeText(downLinkStr[ifaceDataRow]);
                    downLinkLabel[row]->setVisible (true);
                }
            }
        }
        else
        {
            bgTile4->resetGeometry (SCALE_WIDTH(1036), SCALE_HEIGHT(230));
            interfaceHeaderStr[ADV_DETAIL_LAN2]->changeText (interfaceStr[6]);
            interfaceHeaderStr[ADV_DETAIL_BROADBAND]->changeText (interfaceStr[7]);

            for(quint8 row = 0; row < MAX_ADV_DETAIL_INTERFACE_HEADER; row++)
            {
                interfaceHeader[row]->setVisible (true);
                interfaceHeaderStr[row]->setVisible (true);
            }

            for(quint8 row = 0; row < MAX_ADV_DETAIL_INTERFACE; row++)
            {
                for(quint8 col = 0; col < MAX_ADV_DETAIL_INTERFACE_FIELD; col++)
                {
                    interfaceField[row][col]->setVisible (true);
                }

                networkStatusLabel[row]->changeText(networkStatusStr[row]);
                networkStatusLabel[row]->setVisible (true);
                ipv4AddressLabel[row]->changeText(ipv4AddrStr[row]);
                ipv4AddressLabel[row]->setVisible (true);
                ipv6AddressLabel[row]->changeText(ipv6AddrStr[row]);
                ipv6AddressLabel[row]->setVisible (true);
                upLinkLabel[row]->changeText(upLinkStr[row]);
                upLinkLabel[row]->setVisible (true);
                downLinkLabel[row]->changeText(downLinkStr[row]);
                downLinkLabel[row]->setVisible (true);
            }
        }

        bgTile5->setVisible (true);
        networkStreamStr->setVisible (true);
        livePbElement[0]->changeValue (QString("%1").arg (liveStream));
        livePbElement[0]->setVisible (true);
        livePbElement[1]->changeValue (QString("%1").arg (pbStream));
        livePbElement[1]->setVisible (true);

        bgTile6->setVisible (true);
        systemStrHeader->setVisible (true);
        systemFieldReadOnly[0]->changeValue (QString("%1 %").arg (cpuUseValue));
        systemFieldReadOnly[0]->setVisible (true);
        systemFieldReadOnly[1]->changeValue (QString("%1 ").arg (daysParse[0]) + Multilang("Day") + " " +
                QString("%1 ").arg (hourParse[0]) + Multilang("Hrs") + "  " + QString("%1 ").arg (minuteParse[0]) + Multilang("mins"));
        systemFieldReadOnly[1]->setVisible (true);

        /* Display CPU Temp for Rockchip & Hisilicon NVRs Only */
        if (devTable.productVariant >= NVRX_SUPPORT_START_VARIANT)
        {
            //systemFieldReadOnly[0]->updateElementPos(SCALE_WIDTH(12));
            systemFieldReadOnly[2]->changeValue (QString("%1 °C").arg (cpuTempValue));
            systemFieldReadOnly[2]->setVisible (true);
        }
        else if(currentDevName != "")
        {
            systemFieldReadOnly[2]->setVisible (false);
            systemFieldReadOnly[0]->updateElementPos(-1);
        }

        bgTile7->setVisible (true);
        recRateStr->setVisible (true);
        recRate[0]->changeValue (QString("%1 ").arg (recStatusPerHr) + Multilang("MB"));
        recRate[0]->setVisible (true);
        recRate[1]->changeValue(QString("%1 ").arg (recStatusPerDay) + Multilang("MB"));
        recRate[1]->setVisible (true);

        bgTile8->setVisible (true);
        recStatusStr->setVisible (true);
        recStatus[0]->changeValue (activeRecStream);
        recStatus[0]->setVisible (true);
        if((recStatusPerHr.toInt () == 0) && (recStatusPerDay.toInt () == 0))
        {
            recStatus[1]->changeValue ("");
        }
        else
        {
            recStatus[1]->changeValue (QString("%1 ").arg (daysParse[1]) + Multilang("Day") + " " +
                    QString("%1 ").arg (hourParse[1]) + Multilang("Hrs") + "  " + QString("%1 ").arg (minuteParse[1]) + Multilang("mins"));
        }
        recStatus[1]->setVisible (true);

        recStatus[2]->changeValue((m_internetConn == true) ? "Connected" : "Disconnected");
        recStatus[2]->setVisible(true);
    }
    else
    {
        bgTile4->setVisible (false);

        for(quint8 row = 0; row < MAX_ADV_DETAIL_INTERFACE_HEADER; row++)
        {
            interfaceHeader[row]->setVisible (false);
            interfaceHeaderStr[row]->setVisible (false);
        }

        for(quint8 row = 0; row < MAX_ADV_DETAIL_INTERFACE; row++)
        {
            for(quint8 col = 0; col < MAX_ADV_DETAIL_INTERFACE_FIELD; col++)
            {
                interfaceField[row][col]->setVisible (false);
            }

            networkStatusLabel[row]->setVisible (false);
            ipv4AddressLabel[row]->setVisible (false);
            ipv6AddressLabel[row]->setVisible (false);
            upLinkLabel[row]->setVisible (false);
            downLinkLabel[row]->setVisible (false);
        }

        bgTile5->setVisible (false);
        networkStreamStr->setVisible (false);
        livePbElement[0]->setVisible (false);
        livePbElement[1]->setVisible (false);

        bgTile6->setVisible (false);
        systemStrHeader->setVisible (false);
        systemFieldReadOnly[0]->setVisible (false);
        systemFieldReadOnly[1]->setVisible (false);
        systemFieldReadOnly[2]->setVisible (false);

        bgTile7->setVisible (false);
        recRateStr->setVisible (false);
        recRate[0]->setVisible (false);
        recRate[1]->setVisible (false);


        bgTile8->setVisible (false);
        recStatusStr->setVisible (false);
        recStatus[0]->setVisible (false);
        recStatus[1]->setVisible (false);
        recStatus[2]->setVisible(false);
    }
}

void AdvanceDetails::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    if ((deviceName != currentDevName) || (param->msgType != MSG_SET_CMD))
    {
        return;
    }

    payloadLib->parseDevCmdReply(true, param->payload);
    switch(param->deviceStatus)
    {
        case CMD_SUCCESS:
        {
            if (param->cmdType == ADVANCE_STS)
            {
                // fill LAN parameter
                networkStatusStr[ADV_DETAIL_INTERFACE_LAN1] = lanstateStr[payloadLib->getCnfgArrayAtIndex(0).toInt ()];
                ipv4AddrStr[ADV_DETAIL_INTERFACE_LAN1] = payloadLib->getCnfgArrayAtIndex(1).toString ();
                upLinkStr[ADV_DETAIL_INTERFACE_LAN1] = payloadLib->getCnfgArrayAtIndex(2).toString ();
                downLinkStr[ADV_DETAIL_INTERFACE_LAN1] = payloadLib->getCnfgArrayAtIndex(3).toString ();
                ipv6AddrStr[ADV_DETAIL_INTERFACE_LAN1] = payloadLib->getCnfgArrayAtIndex(22).toString ();

                networkStatusStr[ADV_DETAIL_INTERFACE_LAN2] = lanstateStr[payloadLib->getCnfgArrayAtIndex(4).toInt ()];
                ipv4AddrStr[ADV_DETAIL_INTERFACE_LAN2] = payloadLib->getCnfgArrayAtIndex(5).toString ();
                upLinkStr[ADV_DETAIL_INTERFACE_LAN2] = payloadLib->getCnfgArrayAtIndex(6).toString ();
                downLinkStr[ADV_DETAIL_INTERFACE_LAN2] = payloadLib->getCnfgArrayAtIndex(7).toString ();
				ipv6AddrStr[ADV_DETAIL_INTERFACE_LAN2] = payloadLib->getCnfgArrayAtIndex(23).toString ();

                // fill Broadband parameter
                networkStatusStr[ADV_DETAIL_INTERFACE_BROADBAND] = modemstateStr[payloadLib->getCnfgArrayAtIndex(8).toInt ()];
                if (networkStatusStr[ADV_DETAIL_INTERFACE_BROADBAND] != modemstateStr[2])
                {
                    ipv4AddrStr[ADV_DETAIL_INTERFACE_BROADBAND] = "";
                    ipv6AddrStr[ADV_DETAIL_INTERFACE_BROADBAND] = "";
                }
                else
                {
                    ipv4AddrStr[ADV_DETAIL_INTERFACE_BROADBAND] = payloadLib->getCnfgArrayAtIndex(9).toString ();
                    ipv6AddrStr[ADV_DETAIL_INTERFACE_BROADBAND] = payloadLib->getCnfgArrayAtIndex(24).toString ();
                }
                upLinkStr[ADV_DETAIL_INTERFACE_BROADBAND] = payloadLib->getCnfgArrayAtIndex(10).toString ();
                downLinkStr[ADV_DETAIL_INTERFACE_BROADBAND] = payloadLib->getCnfgArrayAtIndex(11).toString ();

                liveStream = payloadLib->getCnfgArrayAtIndex(12).toUInt ();
                pbStream = payloadLib->getCnfgArrayAtIndex(13).toUInt ();

                cpuUseValue = payloadLib->getCnfgArrayAtIndex(14).toInt ();
                convertSecToHMS (payloadLib->getCnfgArrayAtIndex(15).toInt (), 0);

                activeRecStream = payloadLib->getCnfgArrayAtIndex(16).toString ();
                convertSecToHMS (payloadLib->getCnfgArrayAtIndex(17).toInt (), 1);
                recStatusPerHr = payloadLib->getCnfgArrayAtIndex(18).toString ();
                recStatusPerDay = payloadLib->getCnfgArrayAtIndex(19).toString ();
                cpuTempValue = payloadLib->getCnfgArrayAtIndex(20).toInt ();
                m_internetConn = payloadLib->getCnfgArrayAtIndex(21).toBool();

                getAdvanceDetailFrmDev (currentDevName, ADVANCE_STS_CAMERA);
            }
            else if (param->cmdType == ADVANCE_STS_CAMERA)
            {
                quint16 fieldIndex = 0;
                memset(channelFpsValue, 0, sizeof(channelFpsValue));
                memset(channelGopValue, 0, sizeof(channelGopValue));

                /* Store data for main stream */
                for(quint8 index = 0; index < CAMERAS_MAX_V1; index++)
                {
                    channelFpsValue[LIVE_STREAM_TYPE_MAIN][index] = payloadLib->getCnfgArrayAtIndex(fieldIndex++).toInt();
                }

                for(quint8 index = 0; index < CAMERAS_MAX_V1; index++)
                {
                    channelGopValue[LIVE_STREAM_TYPE_MAIN][index] = payloadLib->getCnfgArrayAtIndex(fieldIndex++).toInt();
                }

                do
                {
                    /* Store data for sub stream if it is available in payload */
                    if (false == payloadLib->getCnfgArrayAtIndex(fieldIndex++).toBool())
                    {
                        break;
                    }

                    for(quint8 index = 0; index < CAMERAS_MAX_V1; index++)
                    {
                        channelFpsValue[LIVE_STREAM_TYPE_SUB][index] = payloadLib->getCnfgArrayAtIndex(fieldIndex++).toInt();
                    }

                    for(quint8 index = 0; index < CAMERAS_MAX_V1; index++)
                    {
                        channelGopValue[LIVE_STREAM_TYPE_SUB][index] = payloadLib->getCnfgArrayAtIndex(fieldIndex++).toInt();
                    }

                    /* Store data of 65 to 96 cameras if it is available in payload */
                    if (false == payloadLib->getCnfgArrayAtIndex(fieldIndex++).toBool())
                    {
                        break;
                    }

                    for(quint8 index = CAMERAS_MAX_V1; index < MAX_CAMERAS; index++)
                    {
                        channelFpsValue[LIVE_STREAM_TYPE_MAIN][index] = payloadLib->getCnfgArrayAtIndex(fieldIndex++).toInt();
                    }

                    for(quint8 index = CAMERAS_MAX_V1; index < MAX_CAMERAS; index++)
                    {
                        channelGopValue[LIVE_STREAM_TYPE_MAIN][index] = payloadLib->getCnfgArrayAtIndex(fieldIndex++).toInt();
                    }

                    for(quint8 index = CAMERAS_MAX_V1; index < MAX_CAMERAS; index++)
                    {
                        channelFpsValue[LIVE_STREAM_TYPE_SUB][index] = payloadLib->getCnfgArrayAtIndex(fieldIndex++).toInt();
                    }

                    for(quint8 index = CAMERAS_MAX_V1; index < MAX_CAMERAS; index++)
                    {
                        channelGopValue[LIVE_STREAM_TYPE_SUB][index] = payloadLib->getCnfgArrayAtIndex(fieldIndex++).toInt();
                    }

                }while(0);

                responseStatus = true;
                resetVisibleElements();
                processBar->unloadProcessBar();
            }
        }
        break;

        default:
        {
            if ((param->cmdType == ADVANCE_STS) || (param->cmdType == ADVANCE_STS_CAMERA))
            {
                responseStatus = false;
                clearDetails();
                processBar->unloadProcessBar();
                infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
            }
        }
        break;
    }

    if(!refreshInterval->isActive())
    {
        refreshInterval->start();
    }
}

void AdvanceDetails::setDefaultFocus()
{
    m_currElement = ADV_DETAIL_SPINBOX;

    if(!(infoPage->isVisible ()))
    {
        m_elementList[m_currElement]->forceActiveFocus ();
    }
}

void AdvanceDetails::showAdvDetail (QString devName, bool forceCamFieldToShow)
{
    if(forceCamFieldToShow)
    {
        isCamField = true;
        cameraTab->setShowClickedImage (true);
        systemTab->setShowClickedImage (false);
    }
    else
    {
        isCamField = false;
        cameraTab->setShowClickedImage (false);
        systemTab->setShowClickedImage (true);
    }

    currCamPage = totalPages = 0;
    responseStatus = false;
    resetVisibleElements();
    deviceDropDown->setCurrValue(applController->GetDispDeviceName(devName));
    devNameChanged(devName);
}

void AdvanceDetails::devNameChanged (QString devName)
{
    responseStatus = false;
    if(refreshInterval->isActive())
    {
        refreshInterval->stop();
    }

    if(false == applController->GetDeviceInfo(devName, devTable))
    {
        currCamPage = totalPages = 0;
        devTable.totalCams = 8;
        responseStatus = false;
        resetVisibleElements();
        infoPage->loadInfoPage (ValidationMessage::getDeviceResponceMessage(CMD_DEV_DISCONNECTED));
    }
    else
    {
        quint16 cameraInPage = (MAX_BGTILES * MAX_CAMERA_IN_ONE_BG);
        totalPages = ((devTable.totalCams%cameraInPage) == 0) ? (devTable.totalCams/cameraInPage) : (devTable.totalCams/cameraInPage) + 1;
        processBar->loadProcessBar();
        getAdvanceDetailFrmDev(devName, ADVANCE_STS);
    }
    currentDevName = devName;
}

void AdvanceDetails::showEvent (QShowEvent *event)
{
    QWidget::showEvent (event);

    if(!(infoPage->isVisible ()))
    {
        m_elementList[m_currElement]->forceActiveFocus ();
    }
}

void AdvanceDetails::navigationKeyPressed (QKeyEvent *event)
{
    event->accept();
}

void AdvanceDetails::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    m_elementList[ADV_DETAIL_CLOSE_BUTTON]->forceActiveFocus ();
    m_mainCloseButton->takeEnterKeyAction ();
}

void AdvanceDetails::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void AdvanceDetails::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void AdvanceDetails::takeLeftKeyAction()
{
    do
    {
        m_currElement = (m_currElement - 1 + MAX_ADV_DETAIL_CONTROL) % MAX_ADV_DETAIL_CONTROL;
    }while(!m_elementList[m_currElement]->getIsEnabled());

    m_elementList[m_currElement]->forceActiveFocus();
}


void AdvanceDetails::takeRightKeyAction()
{
    do
    {
        m_currElement = (m_currElement + 1) % MAX_ADV_DETAIL_CONTROL;
    }while(!m_elementList[m_currElement]->getIsEnabled());

    m_elementList[m_currElement]->forceActiveFocus();
}

void AdvanceDetails::clearDetails ()
{
    if(isCamField)
    {
        for(quint8 bgIndex = 0; bgIndex < MAX_BGTILES; bgIndex++)
        {
            for(quint8 row = 0; row < 2; row++)
            {
                for(quint8 col = 0; col < MAX_CAMERA_IN_ONE_BG; col++)
                {
                    channelFieldStr[row*bgIndex][col]->changeText ("");
                }
            }
        }

        prevButton->setIsEnabled (false);
        prevButton->setVisible (false);
        nextButton->setIsEnabled (false);
        nextButton->setVisible (false);

        for(quint8 index = 0; index < MAX_ADV_STS_PAGE; index++)
        {
            m_PageNumberLabel[index]->setVisible (false);
            m_PageNumberLabel[index]->setIsEnabled (false);
        }
    }
    else
    {
        for(quint8 row = 0; row < MAX_ADV_DETAIL_INTERFACE; row++)
        {
            networkStatusLabel[row]->changeText ("");
            ipv4AddressLabel[row]->changeText ("");
            ipv6AddressLabel[row]->changeText ("");
            upLinkLabel[row]->changeText ("");
            downLinkLabel[row]->changeText ("");
        }

        livePbElement[0]->changeValue ("");
        livePbElement[1]->changeValue ("");

        systemFieldReadOnly[0]->changeValue ("");
        systemFieldReadOnly[1]->changeValue ("");
        systemFieldReadOnly[2]->changeValue ("");

        recRate[0]->changeValue ("");
        recRate[1]->changeValue ("");

        recStatus[0]->changeValue ("");
        recStatus[1]->changeValue ("");
        recStatus[2]->changeValue("");
    }
}

void AdvanceDetails::slotUpadateCurrentElement (int index)
{
    m_currElement = index;
}

void AdvanceDetails::slotInfoPageCnfgBtnClick(int)
{
    m_currElement = ADV_DETAIL_SPINBOX;
    m_elementList[m_currElement]->forceActiveFocus ();
}

void AdvanceDetails::slotRefreshButtonClicked()
{
    devNameChanged(currentDevName);
}

void AdvanceDetails::slotBackButtonClicked(int)
{
    emit sigPrevPage (currentDevName, isCamField);
}

void AdvanceDetails::slotSpinBoxValueChange(QString str,quint32)
{
    str = applController->GetRealDeviceName(str);
    if(str != currentDevName)
    {
        currCamPage = 0;
        devNameChanged(str);
    }
}

void AdvanceDetails::slotTabSelected(int index)
{
    if(index == 0)
    {
        isCamField = true;
        currCamPage = 0;
        systemTab->setShowClickedImage (false);
    }
    else
    {
        isCamField = false;
        cameraTab->setShowClickedImage (false);
    }
    resetVisibleElements ();
}

void AdvanceDetails::slotPrevNextCameraClicked (int index)
{
    if(index == ADV_DETAIL_NEXT_BTN)
    {
        if (currCamPage != (totalPages - 1))
        {
            currCamPage ++;
        }
        nextPageSelected = true;
    }
    else
    {
        if(currCamPage > 0)
        {
            currCamPage --;
        }
        nextPageSelected = false;
    }

    resetVisibleElements ();
}

void AdvanceDetails::slotPageNumberButtonClick(QString str)
{
    quint8 tempPageNum = ((quint8)str.toUInt () - 1);
    nextPageSelected = (tempPageNum < currCamPage) ? false : true;
    currCamPage = tempPageNum;
    resetVisibleElements ();
}

void AdvanceDetails::slotDropDownListDestroyed()
{
    if((infoPage->isVisible ()))
    {
        infoPage->forceActiveFocus();
    }
}

void AdvanceDetails::updateDeviceList(void)
{
    QMap<quint8, QString> deviceMapList;
    applController->GetDevNameDropdownMapList(deviceMapList);

    /* Check if selected device found in new updated list or not. It is possible that index of that device name may changed.
     * Hence update device list with current device index */
    for (quint8 deviceIndex = 0; deviceIndex < deviceMapList.count(); deviceIndex++)
    {
        if (deviceDropDown->getCurrValue() == deviceMapList.value(deviceIndex))
        {
            deviceDropDown->setNewList(deviceMapList, deviceIndex);
            return;
        }
    }

    /* If selected device is local device then it will update the device name only otherwise it will clear the data and will select the local device */
    deviceDropDown->setNewList(deviceMapList, 0);
    slotSpinBoxValueChange(deviceMapList.value(0), ADV_DETAIL_SPINBOX);
}
