#include "HDDGroup.h"

#define HEADING_MARGIN              SCALE_WIDTH(25)
#define WIZARD_BG_TILE_WIDTH        SCALE_WIDTH(968)
#define WARNING_REC_BITRATE         "Note: Maximum recommended bitrate per HDD group for recording is 256 Mbps"
#define WARNING_STORAGE_DEV         "Storage device not detected"
#define BGTILE_HDD_GRP_CHECKBOX     BGTILE_MEDIUM_SIZE_WIDTH - SCALE_WIDTH(10)
#define CNFG_FRM_INDEX              1
#define CNFG_FRM_FIELD              1

#define GET_VOLUME_MASK_BIT(volMask, volume)   ((volMask >> volume) & 1ULL)
#define SET_VOLUME_MASK_BIT(volMask, volume)   volMask |= ((quint32)1 << volume)
#define CLR_VOLUME_MASK_BIT(volMask, volume)   volMask &= ~((quint32)1 << volume)

#define STRING_BEFORE_GRP(str)  ((str).left((str).indexOf(" (GRP")).trimmed())

typedef enum
{
    HDD_GROUP_DROP_DOWN,
    HDD_GROUP_SEL_ALL_VOLUME,
    HDD_GROUP_VOLUME_LIST,
    HDD_GROUP_SEL_ALL_CAMERA = (HDD_GROUP_VOLUME_LIST + MAX_LOGICAL_VOLUME_POSSIBLE),
    HDD_GROUP_CAMERA_LIST,
    HDD_GROUP_CAMERA_PREV_BTN = (HDD_GROUP_CAMERA_LIST + MAX_CAM_ON_SINGLE_PAGE),
    HDD_GROUP_CAMERA_PAGE_NUM_BTN,
    HDD_GROUP_CAMERA_NEXT_BTN = (HDD_GROUP_CAMERA_PAGE_NUM_BTN + MAX_PAGE_NUMBER),
    MAX_HDD_GROUP_CONTROL
}HDD_GROUP_CONTROL_e;

typedef enum
{
    STORAGE_ALLOCATION_VOLUME_MASK_GROUP_START = 0,
    STORAGE_ALLOCATION_VOLUME_MASK_GROUP_END = (STORAGE_ALLOCATION_VOLUME_MASK_GROUP_START + STORAGE_ALLOCATION_GROUP_MAX - 1),
    STORAGE_ALLOCATION_CAMERA_MASK_GROUP_START,
    STORAGE_ALLOCATION_CAMERA_MASK_GROUP_END = (STORAGE_ALLOCATION_CAMERA_MASK_GROUP_START + (STORAGE_ALLOCATION_GROUP_MAX * CAMERA_MASK_MAX) - 1),
    MAX_STORAGE_ALLOCATION_CFG_FIELD,
}STORAGE_ALLOCATION_CFG_FIELD_e;

typedef enum
{
    LOG_STS_NORMAL,
    LOG_STS_CREATING,
    LOG_STS_FORMATING,
    LOG_STS_FULL,
    LOG_STS_FAULT,
    LOG_STS_INCOMPLETE,
    LOG_STS_CLEANINGUP,
    LOG_STS_NODISK,
    LOG_STS_READ_ONLY,
    LOG_STS_NONE,
    MAX_LOG_STATE
}HDD_GROUP_LOG_STATUS_RES_e;

typedef enum
{
    HDD_GROUP_MNG_HDD_GRP,
    HDD_GROUP_MNG_SEL_VOLUME,
    HDD_GROUP_MNG_SEL_CAMERA,
    HDD_GROUP_MNG_SEL_ALL_LBL,
    HDD_GROUP_MNG_PREV_LBL,
    HDD_GROUP_MNG_NEXT_LBL,
    MAX_HDD_GROUP_MNG_STRG
}HDD_GROUP_MNG_STRG_e;

static const QString hddGroupManagmentStrings[MAX_HDD_GROUP_MNG_STRG] =
{
    "HDD Group",
    "Select Volume(s)",
    "Select Cameras",
    "All",
    "Previous",
    "Next",
};

static const QString groupLableStrings[STORAGE_ALLOCATION_GROUP_MAX] =
{
    " (GRP 1)", " (GRP 2)", " (GRP 3)", " (GRP 4)", " (GRP 5)", " (GRP 6)", " (GRP 7)", " (GRP 8)"
};

static const QStringList driveListName = QStringList()
        << "Single Disk Volume: Drive 1"
        << "Single Disk Volume: Drive 2"
        << "Single Disk Volume: Drive 3"
        << "Single Disk Volume: Drive 4"
        << "Single Disk Volume: Drive 5"
        << "Single Disk Volume: Drive 6"
        << "Single Disk Volume: Drive 7"
        << "Single Disk Volume: Drive 8";

static const QStringList stripRaidVolumeNameForPartitions = QStringList()
        << "Striping Disk Volume 0 : Drive 1 2"
        << "Striping Disk Volume 1 : Drive 1 2"
        << "Striping Disk Volume 2 : Drive 1 2"
        << "Striping Disk Volume 3 : Drive 1 2"
        << "Striping Disk Volume 0 : Drive 3 4"
        << "Striping Disk Volume 1 : Drive 3 4"
        << "Striping Disk Volume 2 : Drive 3 4"
        << "Striping Disk Volume 3 : Drive 3 4"
        << "Striping Disk Volume 0 : Drive 5 6"
        << "Striping Disk Volume 1 : Drive 5 6"
        << "Striping Disk Volume 2 : Drive 5 6"
        << "Striping Disk Volume 3 : Drive 5 6"
        << "Striping Disk Volume 0 : Drive 7 8"
        << "Striping Disk Volume 1 : Drive 7 8"
        << "Striping Disk Volume 2 : Drive 7 8"
        << "Striping Disk Volume 3 : Drive 7 8";

static const QStringList mirrorRaidVolumeNameForPartitions = QStringList()
        << "Mirroring Disk Volume 0 : Drive 1 2"
        << "Mirroring Disk Volume 1 : Drive 1 2"
        << "Mirroring Disk Volume 2 : Drive 1 2"
        << "Mirroring Disk Volume 3 : Drive 1 2"
        << "Mirroring Disk Volume 0 : Drive 3 4"
        << "Mirroring Disk Volume 1 : Drive 3 4"
        << "Mirroring Disk Volume 2 : Drive 3 4"
        << "Mirroring Disk Volume 3 : Drive 3 4"
        << "Mirroring Disk Volume 0 : Drive 5 6"
        << "Mirroring Disk Volume 1 : Drive 5 6"
        << "Mirroring Disk Volume 2 : Drive 5 6"
        << "Mirroring Disk Volume 3 : Drive 5 6"
        << "Mirroring Disk Volume 0 : Drive 7 8"
        << "Mirroring Disk Volume 1 : Drive 7 8"
        << "Mirroring Disk Volume 2 : Drive 7 8"
        << "Mirroring Disk Volume 3 : Drive 7 8";

static const QStringList raid5with3DiskVolNameForPartitions = QStringList()
        << "RAID 5 Volume 0 : Drive 1 2 3"
        << "RAID 5 Volume 1 : Drive 1 2 3"
        << "RAID 5 Volume 2 : Drive 1 2 3"
        << "RAID 5 Volume 3 : Drive 1 2 3"
        << "RAID 5 Volume 0 : Drive 5 6 7"
        << "RAID 5 Volume 1 : Drive 5 6 7"
        << "RAID 5 Volume 2 : Drive 5 6 7"
        << "RAID 5 Volume 3 : Drive 5 6 7";

static const QStringList raid5with4DiskVolNameForPartitions = QStringList()
        << "RAID 5 Volume 0 : Drive 1 2 3 4"
        << "RAID 5 Volume 1 : Drive 1 2 3 4"
        << "RAID 5 Volume 2 : Drive 1 2 3 4"
        << "RAID 5 Volume 3 : Drive 1 2 3 4"
        << "RAID 5 Volume 0 : Drive 5 6 7 8"
        << "RAID 5 Volume 1 : Drive 5 6 7 8"
        << "RAID 5 Volume 2 : Drive 5 6 7 8"
        << "RAID 5 Volume 3 : Drive 5 6 7 8";

static const QStringList raid10VolNameForPartitions = QStringList()
        << "RAID 10 Volume 0 : Drive 1 2 3 4"
        << "RAID 10 Volume 1 : Drive 1 2 3 4"
        << "RAID 10 Volume 2 : Drive 1 2 3 4"
        << "RAID 10 Volume 3 : Drive 1 2 3 4"
        << "RAID 10 Volume 0 : Drive 5 6 7 8"
        << "RAID 10 Volume 1 : Drive 5 6 7 8"
        << "RAID 10 Volume 2 : Drive 5 6 7 8"
        << "RAID 10 Volume 3 : Drive 5 6 7 8";

static const QStringList networkdriveList = QStringList()
        << "Network Drive 1"
        << "Network Drive 2";

HDDGroup::HDDGroup(QString devName, QString subHeadStr, QWidget *parent, WIZARD_PAGE_INDEXES_e pageId)
    : WizardCommon(parent, pageId)
{
    memset(&storageAllocationInfo, 0, sizeof(storageAllocationInfo));
    memset(&isCameraSelected, 0,  sizeof(isCameraSelected));
    memset(&isVolumeSelected, 0,  sizeof(isVolumeSelected));
    currentElement = 0;
    volListSize = 0;
    currentPageNum = 0;
    currentGroupSelected = 0;
    totalCameraSelected = 0;
    tmpGrpIdx = 0;
    tmpIdx = 0;
    nextPageSelected = false;
    isAllChkBoxClicked = false;
    currDevName = devName;
    applController = ApplController::getInstance();
    applController->GetDeviceInfo(currDevName, devTableInfo);

    for(quint8 index = 0 ; index < MAX_LOGICAL_VOLUME_POSSIBLE; index++)
    {
        INIT_OBJ(m_volumeListCheckBox[index]);
        INIT_OBJ(m_volumeListLabel[index]);
    }

    fillCameraList();
    totalCamera = cameraList.size();
    totalPages = (totalCamera / MAX_CAM_ON_SINGLE_PAGE) + ((totalCamera % MAX_CAM_ON_SINGLE_PAGE) ? 1 : 0);

    createDefaultComponents(subHeadStr);
    getConfig();
    this->show();
}

HDDGroup::~HDDGroup()
{
    m_configResponse.clear();
    DELETE_OBJ(payloadLib);
    DELETE_OBJ(m_hddGroupHeading);
    DELETE_OBJ(m_hddGroupDropDownLabel);

    if(IS_VALID_OBJ(m_hddGroupDropDown))
    {
        disconnect(m_hddGroupDropDown,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_hddGroupDropDown,
                 SIGNAL(sigValueChanged(QString, quint32)),
                 this,
                 SLOT(slotDropdownValueChanged(QString, quint32)));
        DELETE_OBJ(m_hddGroupDropDown);
    }

    DELETE_OBJ(m_volumeSelectHeading);

    if(IS_VALID_OBJ(m_allVolumeChecklist))
    {
        disconnect(m_allVolumeChecklist,
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_allVolumeChecklist,
                 SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                 this,
                 SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));
        DELETE_OBJ(m_allVolumeChecklist);
    }

    for(quint8 index = 0 ; index < MAX_LOGICAL_VOLUME_POSSIBLE; index++)
    {
        if(IS_VALID_OBJ(m_volumeListCheckBox[index]))
        {
            disconnect(m_volumeListCheckBox[index],
                      SIGNAL(sigUpdateCurrentElement(int)),
                      this,
                      SLOT(slotUpdateCurrentElement(int)));
            disconnect(m_volumeListCheckBox[index],
                      SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                      this,
                      SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));
            DELETE_OBJ(m_volumeListCheckBox[index]);
        }
        DELETE_OBJ(m_volumeListLabel[index]);
    }

    DELETE_OBJ(m_cameraSelectHeading);

    if(IS_VALID_OBJ(m_allCameraChecklist))
    {
        disconnect(m_allCameraChecklist,
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
        disconnect(m_allCameraChecklist,
                 SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                 this,
                 SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));
        DELETE_OBJ(m_allCameraChecklist);
    }

    for(quint8 index = 0 ; index < MAX_CAM_ON_SINGLE_PAGE; index++)
    {
        if(IS_VALID_OBJ(m_cameraListCheckBox[index]))
        {
            disconnect(m_cameraListCheckBox[index],
                      SIGNAL(sigUpdateCurrentElement(int)),
                      this,
                      SLOT(slotUpdateCurrentElement(int)));
            disconnect(m_cameraListCheckBox[index],
                      SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                      this,
                      SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));
            DELETE_OBJ(m_cameraListCheckBox[index]);
        }

        DELETE_OBJ(m_cameraListLabel[index]);
    }

    if(IS_VALID_OBJ(m_cameraPrevButton))
    {
        disconnect(m_cameraPrevButton,
                 SIGNAL(sigButtonClick(int)),
                 this,
                 SLOT(slotButtonClick(int)));
        disconnect(m_cameraPrevButton,
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_cameraPrevButton);
    }

    for(quint8 index = 0; index < MAX_PAGE_NUMBER; index++)
    {
        if(IS_VALID_OBJ(m_cameraPageNumberLabel[index]))
        {
            disconnect(m_cameraPageNumberLabel[index],
                    SIGNAL(sigMousePressClick(QString)),
                    this,
                    SLOT(slotPageNumberButtonClick(QString)));
            disconnect(m_cameraPageNumberLabel[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
            DELETE_OBJ(m_cameraPageNumberLabel[index]);
        }
    }

    if(IS_VALID_OBJ(m_cameraNextButton))
    {
        disconnect(m_cameraNextButton,
                 SIGNAL(sigButtonClick(int)),
                 this,
                 SLOT(slotButtonClick(int)));
        disconnect(m_cameraNextButton,
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
        DELETE_OBJ(m_cameraNextButton);
    }

    DELETE_OBJ(m_footnoteLabelBitRate);
    DELETE_OBJ(m_elementHeadingBitRate);

    DELETE_OBJ(m_footnoteLabelNoDevice);
    DELETE_OBJ(m_elementHeadingNoDevice);

    if (IS_VALID_OBJ(infoPage))
    {
        disconnect(infoPage,
                 SIGNAL(sigInfoPageCnfgBtnClick(int)),
                 this,
                 SLOT(slotInfoPageBtnclick(int)));
        DELETE_OBJ(infoPage);
    }
}

void HDDGroup::createDefaultComponents(QString subHeadStr)
{
    payloadLib = new PayloadLib();

    m_hddGroupHeading = new TextLabel((SCALE_WIDTH(485)),
                                      (SCALE_HEIGHT(23)),
                                      SCALE_FONT(SUB_HEADING_FONT_SIZE),
                                      subHeadStr,
                                      this,
                                      HIGHLITED_FONT_COLOR,
                                      NORMAL_FONT_FAMILY,
                                      ALIGN_START_X_CENTRE_Y,
                                      0,
                                      false,
                                      BGTILE_LARGE_SIZE_WIDTH,
                                      0);

    m_hddGroupDropDownLabel = new ElementHeading(SCALE_WIDTH(55), SCALE_HEIGHT(50),
                                           WIZARD_BG_TILE_WIDTH,
                                           BGTILE_HEIGHT,
                                           hddGroupManagmentStrings[HDD_GROUP_MNG_HDD_GRP],
                                           COMMON_LAYER,
                                           this,
                                           false,
                                           HEADING_MARGIN + SCALE_WIDTH(335));

    hddGroupList.clear();
    for(quint8 index = 0; index < STORAGE_ALLOCATION_GROUP_MAX; index++)
    {
        hddGroupList.insert(index, QString::number(index + 1));
    }

    m_hddGroupDropDown = new DropDown(m_hddGroupDropDownLabel->x() + SCALE_WIDTH(480),
                                      m_hddGroupDropDownLabel->y(),
                                      BGTILE_LARGE_SIZE_WIDTH,
                                      BGTILE_HEIGHT,
                                      HDD_GROUP_DROP_DOWN,
                                      DROPDOWNBOX_SIZE_114,
                                      "",
                                      hddGroupList,
                                      this,
                                      "",
                                      false,
                                      0,
                                      NO_LAYER);

    connect(m_hddGroupDropDown,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));
    connect(m_hddGroupDropDown,
             SIGNAL(sigValueChanged(QString, quint32)),
             this,
             SLOT(slotDropdownValueChanged(QString, quint32)));

    m_volumeSelectHeading = new ElementHeading(m_hddGroupDropDownLabel->x(), m_hddGroupDropDownLabel->y() +
                                               m_hddGroupDropDownLabel->height() + SCALE_HEIGHT(10),
                                               BGTILE_HDD_GRP_CHECKBOX,
                                               BGTILE_HEIGHT,
                                               hddGroupManagmentStrings[HDD_GROUP_MNG_SEL_VOLUME],
                                               TOP_LAYER,
                                               this,
                                               false,
                                               HEADING_MARGIN);

    m_allVolumeChecklist = new OptionSelectButton(m_volumeSelectHeading->x(),
                                                  m_volumeSelectHeading->y() + BGTILE_HEIGHT,
                                                  BGTILE_HDD_GRP_CHECKBOX,
                                                  BGTILE_HEIGHT,
                                                  CHECK_BUTTON_INDEX,
                                                  hddGroupManagmentStrings[HDD_GROUP_MNG_SEL_ALL_LBL],
                                                  this,
                                                  MIDDLE_TABLE_LAYER,
                                                  SCALE_WIDTH(40),
                                                  MX_OPTION_TEXT_TYPE_SUFFIX,
                                                  NORMAL_FONT_SIZE,
                                                  HDD_GROUP_SEL_ALL_VOLUME);

     connect (m_allVolumeChecklist,
              SIGNAL(sigUpdateCurrentElement(int)),
              this,
              SLOT(slotUpdateCurrentElement(int)));
     connect (m_allVolumeChecklist,
              SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
              this,
              SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));

    m_cameraSelectHeading = new ElementHeading(m_hddGroupDropDownLabel->x() + BGTILE_HDD_GRP_CHECKBOX + SCALE_WIDTH(15),
                                               m_hddGroupDropDownLabel->y() + m_hddGroupDropDownLabel->height() + SCALE_HEIGHT(10),
                                               BGTILE_HDD_GRP_CHECKBOX,
                                               BGTILE_HEIGHT,
                                               hddGroupManagmentStrings[HDD_GROUP_MNG_SEL_CAMERA],
                                               TOP_LAYER,
                                               this,
                                               false,
                                               HEADING_MARGIN);

    m_allCameraChecklist = new OptionSelectButton(m_cameraSelectHeading->x(),
                                                  (m_cameraSelectHeading->y() + BGTILE_HEIGHT),
                                                  BGTILE_HDD_GRP_CHECKBOX,
                                                  BGTILE_HEIGHT,
                                                  CHECK_BUTTON_INDEX,
                                                  hddGroupManagmentStrings[HDD_GROUP_MNG_SEL_ALL_LBL],
                                                  this,
                                                  MIDDLE_TABLE_LAYER,
                                                  SCALE_WIDTH(40),
                                                  MX_OPTION_TEXT_TYPE_SUFFIX,
                                                  NORMAL_FONT_SIZE,
                                                  HDD_GROUP_SEL_ALL_CAMERA);

     connect (m_allCameraChecklist,
              SIGNAL(sigUpdateCurrentElement(int)),
              this,
              SLOT(slotUpdateCurrentElement(int)));
     connect (m_allCameraChecklist,
              SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
              this,
              SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));

    for(quint8 index = 0 ; index < MAX_CAM_ON_SINGLE_PAGE; index++)
    {
        m_cameraListCheckBox[index] = new OptionSelectButton(m_allCameraChecklist->x(),
                                                            (m_allCameraChecklist->y() + m_allCameraChecklist->height() +
                                                            (index * BGTILE_HEIGHT)),
                                                            BGTILE_HDD_GRP_CHECKBOX,
                                                            BGTILE_HEIGHT,
                                                            CHECK_BUTTON_INDEX,
                                                            "",
                                                            this,
                                                            MIDDLE_TABLE_LAYER,
                                                            SCALE_WIDTH(40),
                                                            MX_OPTION_TEXT_TYPE_SUFFIX,
                                                            NORMAL_FONT_SIZE,
                                                            (HDD_GROUP_CAMERA_LIST + index));

        connect (m_cameraListCheckBox[index],
                  SIGNAL(sigUpdateCurrentElement(int)),
                  this,
                  SLOT(slotUpdateCurrentElement(int)));
        connect (m_cameraListCheckBox[index],
                  SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                  this,
                  SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));

        m_cameraListLabel[index] = new TextLabel(m_allCameraChecklist->x () + SCALE_WIDTH(85),
                                                 m_allCameraChecklist->y() + m_allCameraChecklist->height() +
                                                 (index * BGTILE_HEIGHT + SCALE_HEIGHT(10)),
                                                 NORMAL_FONT_SIZE,
                                                 (index < totalCamera) ? cameraList.value(index) : "",
                                                 this, NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                                 0, 0, SCALE_WIDTH(356));
    }

    m_cameraPrevButton = new ControlButton(PREVIOUS_BUTTON_INDEX,
                                           m_cameraSelectHeading->x(),
                                           m_cameraListCheckBox[MAX_CAM_ON_SINGLE_PAGE - 1]->y() +
                                           m_cameraListCheckBox[MAX_CAM_ON_SINGLE_PAGE - 1]->height(),
                                           BGTILE_HDD_GRP_CHECKBOX,
                                           BGTILE_HEIGHT,
                                           this,
                                           DOWN_LAYER,
                                           SCALE_WIDTH(20),
                                           hddGroupManagmentStrings[HDD_GROUP_MNG_PREV_LBL],
                                           false,
                                           HDD_GROUP_CAMERA_PREV_BTN,
                                           false);

    connect (m_cameraPrevButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));
    connect (m_cameraPrevButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    for(quint8 index = 0; index < MAX_PAGE_NUMBER; index++)
    {
        m_cameraPageNumberLabel[index] = new TextWithBackground((m_cameraPrevButton->x() +
                                                                (SCALE_WIDTH(185) + (index * SCALE_WIDTH(40)))),
                                                                m_cameraPrevButton->y() + SCALE_HEIGHT(5),
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
                                                                (HDD_GROUP_CAMERA_PAGE_NUM_BTN + index));

        connect(m_cameraPageNumberLabel[index],
                SIGNAL(sigMousePressClick(QString)),
                this,
                SLOT(slotPageNumberButtonClick(QString)));
        connect(m_cameraPageNumberLabel[index],
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    }

    for(quint8 index = 0; index < MAX_PAGE_NUMBER; index++)
    {
        m_cameraPageNumberLabel[index]->changeText(QString(" ") + QString("%1").arg (currentPageNum  + 1 + index) + QString(" "));

        if(index == 0)
        {
            m_cameraPageNumberLabel[index]->setBackGroundColor (CLICKED_BKG_COLOR);
            m_cameraPageNumberLabel[index]->changeTextColor (HIGHLITED_FONT_COLOR);
            m_cameraPageNumberLabel[index]->setBold (true);
            m_cameraPageNumberLabel[index]->forceActiveFocus ();
        }
        else
        {
            m_cameraPageNumberLabel[index]->setBackGroundColor (TRANSPARENTKEY_COLOR);
            m_cameraPageNumberLabel[index]->changeTextColor(NORMAL_FONT_COLOR);
            m_cameraPageNumberLabel[index]->setBold (false);
            m_cameraPageNumberLabel[index]->deSelectControl ();
        }

        m_cameraPageNumberLabel[index]->update ();
    }

    if (totalCamera < MAX_CAM_ON_SINGLE_PAGE)
    {
        for(quint8 index = totalCamera; index < MAX_CAM_ON_SINGLE_PAGE; index++)
        {
            m_cameraListCheckBox[index]->setVisible (false);
            m_cameraListCheckBox[index]->setIsEnabled (false);
            m_cameraListLabel[index]->setVisible (false);
            m_cameraListLabel[index]->setIsEnabled (false);
        }
    }

    if(totalPages < MAX_PAGE_NUMBER)
    {
        m_cameraPageNumberLabel[0]->setOffset((m_cameraPageNumberLabel[0]->x() + SCALE_WIDTH(20 * (MAX_PAGE_NUMBER - totalPages))), m_cameraPageNumberLabel[0]->y());

        for(quint8 index = 1; index < MAX_PAGE_NUMBER; index++)
        {
            if (index >= totalPages)
            {
                m_cameraPageNumberLabel[index]->setVisible (false);
                m_cameraPageNumberLabel[index]->setIsEnabled (false);
            }
            else
            {
                m_cameraPageNumberLabel[index]->setOffset((m_cameraPageNumberLabel[index - 1]->x() + m_cameraPageNumberLabel[index - 1]->width()), m_cameraPageNumberLabel[index]->y());
            }
        }
    }

    m_cameraNextButton = new ControlButton(NEXT_BUTTON_INDEX,
                                           m_cameraPrevButton->x() + m_cameraPrevButton->width() - SCALE_WIDTH(90),
                                           m_cameraPrevButton->y(),
                                           BGTILE_HDD_GRP_CHECKBOX,
                                           BGTILE_HEIGHT,
                                           this,
                                           NO_LAYER,
                                           0,
                                           hddGroupManagmentStrings[HDD_GROUP_MNG_NEXT_LBL],
                                           true,
                                           HDD_GROUP_CAMERA_NEXT_BTN);

    connect (m_cameraNextButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));
    connect (m_cameraNextButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    m_elementHeadingBitRate = new ElementHeading(m_hddGroupDropDownLabel->x() + SCALE_WIDTH(180),
                                               m_cameraNextButton->y() + SCALE_HEIGHT(75),
                                               BGTILE_MEDIUM_SIZE_WIDTH,
                                               SCALE_HEIGHT(50),
                                               "",
                                               NO_LAYER,
                                               this);
    QString fontColor = NORMAL_FONT_COLOR;
    QString fontWidth = "" + QString::number(SCALE_WIDTH(15)) + "px";
    QString styl = "ElidedLabel \
    { \
        color: %1; \
        font-size: %2; \
        font-family: %3; \
    }";

    m_elementHeadingBitRate->setStyleSheet(styl.arg(fontColor).arg(fontWidth).arg(NORMAL_FONT_FAMILY));
    m_footnoteLabelBitRate = new ElidedLabel(Multilang(WARNING_REC_BITRATE), m_elementHeadingBitRate);
    m_footnoteLabelBitRate->resize(BGTILE_MEDIUM_SIZE_WIDTH, SCALE_HEIGHT(50));
    m_footnoteLabelBitRate->raise();
    m_footnoteLabelBitRate->show();

    fontColor = DISABLE_FONT_COLOR;
    fontWidth = "" + QString::number(SCALE_WIDTH(17)) + "px";
    styl = "ElidedLabel \
    { \
        color: %1; \
        font-size: %2; \
        font-family: %3; \
    }";

    m_elementHeadingNoDevice = new ElementHeading(m_hddGroupDropDownLabel->x() + SCALE_WIDTH(360),
                                               m_hddGroupDropDownLabel->y() + SCALE_HEIGHT(230),
                                               BGTILE_LARGE_SIZE_WIDTH,
                                               SCALE_HEIGHT(50),
                                               "",
                                               NO_LAYER,
                                               this);

    m_elementHeadingNoDevice->setStyleSheet(styl.arg(fontColor).arg(fontWidth).arg(NORMAL_FONT_FAMILY));
    m_footnoteLabelNoDevice = new ElidedLabel(Multilang(WARNING_STORAGE_DEV), m_elementHeadingNoDevice);
    m_footnoteLabelNoDevice->resize(BGTILE_LARGE_SIZE_WIDTH, SCALE_HEIGHT(50));
    m_footnoteLabelNoDevice->raise();
    m_footnoteLabelNoDevice->show();

    showHideCtrls(false);

    infoPage = new InfoPage(0, 0,
                              SCALE_WIDTH(1145),
                              SCALE_HEIGHT(750),
                              MAX_INFO_PAGE_TYPE,
                              parentWidget(), false, true, SCALE_WIDTH(420));
    connect (infoPage,
             SIGNAL(sigInfoPageCnfgBtnClick(int)),
             this,
             SLOT(slotInfoPageBtnclick(int)));
}

void HDDGroup::createVolumeList()
{
    destroyVolumeList();

    BGTILE_TYPE_e tileType = MIDDLE_TABLE_LAYER;
    for(quint8 index = 0 ; index < volListSize; index++)
    {
        if (index == volListSize - 1)
        {
            tileType = BOTTOM_TABLE_LAYER;
        }

        m_volumeListCheckBox[index] = new OptionSelectButton(m_allVolumeChecklist->x(),
                                                            (m_allVolumeChecklist->y() + m_allVolumeChecklist->height() +
                                                            (index * BGTILE_HEIGHT)),
                                                            BGTILE_HDD_GRP_CHECKBOX,
                                                            BGTILE_HEIGHT,
                                                            CHECK_BUTTON_INDEX,
                                                            "",
                                                            this,
                                                            tileType,
                                                            SCALE_WIDTH(40),
                                                            MX_OPTION_TEXT_TYPE_SUFFIX,
                                                            NORMAL_FONT_SIZE,
                                                            (HDD_GROUP_VOLUME_LIST + index));

        connect (m_volumeListCheckBox[index],
                  SIGNAL(sigUpdateCurrentElement(int)),
                  this,
                  SLOT(slotUpdateCurrentElement(int)));
        connect (m_volumeListCheckBox[index],
                  SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                  this,
                  SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));

        m_volumeListLabel[index] = new TextLabel(m_allVolumeChecklist->x () + SCALE_WIDTH(85),
                                                 m_allVolumeChecklist->y() + m_allVolumeChecklist->height() +
                                                 (index * BGTILE_HEIGHT + SCALE_HEIGHT(10)),
                                                 NORMAL_FONT_SIZE,
                                                 volumeList.value(index),
                                                 this, NORMAL_FONT_COLOR, NORMAL_FONT_FAMILY, ALIGN_START_X_START_Y,
                                                 0, 0, SCALE_WIDTH(356));
    }

    m_elementHeadingBitRate->resetGeometry(m_volumeListCheckBox[volListSize - 1]->x() + SCALE_WIDTH(10),
            m_volumeListCheckBox[volListSize - 1]->y() + SCALE_HEIGHT(60), m_elementHeadingBitRate->getWidth(), SCALE_HEIGHT(50));
}

void HDDGroup::destroyVolumeList()
{
    for(quint8 index = 0 ; index < MAX_LOGICAL_VOLUME_POSSIBLE; index++)
    {
        if(IS_VALID_OBJ(m_volumeListCheckBox[index]))
        {
            disconnect(m_volumeListCheckBox[index],
                      SIGNAL(sigUpdateCurrentElement(int)),
                      this,
                      SLOT(slotUpdateCurrentElement(int)));
            disconnect(m_volumeListCheckBox[index],
                      SIGNAL(sigButtonClicked(OPTION_STATE_TYPE_e,int)),
                      this,
                      SLOT(slotOptionButtonClicked(OPTION_STATE_TYPE_e,int)));
            DELETE_OBJ(m_volumeListCheckBox[index]);
        }
        DELETE_OBJ(m_volumeListLabel[index]);
    }
}

void HDDGroup::showHideCtrls(bool isVisible)
{
    m_hddGroupDropDownLabel->setVisible(isVisible);
    m_hddGroupDropDown->setVisible(isVisible);
    m_volumeSelectHeading->setVisible(isVisible);
    m_allVolumeChecklist->setVisible(isVisible);

    for(quint8 index = 0 ; index < MAX_LOGICAL_VOLUME_POSSIBLE; index++)
    {
        if(IS_VALID_OBJ(m_volumeListCheckBox[index]))
        {
            m_volumeListCheckBox[index]->setVisible(isVisible);
            m_volumeListLabel[index]->setVisible(isVisible);
        }
    }

    m_cameraSelectHeading->setVisible(isVisible);
    m_allCameraChecklist->setVisible(isVisible);

    for(quint8 index = 0 ; index < MAX_CAM_ON_SINGLE_PAGE; index++)
    {

        m_cameraListCheckBox[index]->setVisible(isVisible);
        m_cameraListLabel[index]->setVisible(isVisible);
    }

    m_cameraPrevButton->setVisible(isVisible);
    m_cameraNextButton->setVisible(isVisible);
    m_footnoteLabelBitRate->setVisible(isVisible);
    m_elementHeadingBitRate->setVisible(isVisible);
    m_footnoteLabelNoDevice->setVisible(!isVisible);
    m_elementHeadingNoDevice->setVisible(!isVisible);
}

void HDDGroup::fillCameraList()
{
    cameraList.clear();

    for(quint8 index = 0; index < devTableInfo.totalCams ; index++)
    {
        QString tempStr = applController->GetCameraNameOfDevice(currDevName,index);

        if ((devTableInfo.totalCams > 10) && ((index + 1) < 10))
        {
            cameraList.insert(index, QString(" %1%2%3").arg(index + 1).arg(" : ").arg (tempStr));
        }
        else
        {
            cameraList.insert(index, QString("%1%2%3").arg(index + 1).arg(" : ").arg (tempStr));
        }
    }
}

bool HDDGroup::getVolumeNumber(QString volumeName, qint8 &volumeNumber)
{
    /* Is single disk volume? */
    volumeNumber = driveListName.indexOf(volumeName);
    if (volumeNumber != -1)
    {
        return true;
    }

    /* Is RAID0 volume? */
    volumeNumber = stripRaidVolumeNameForPartitions.indexOf(volumeName);
    if (volumeNumber != -1)
    {
        return true;
    }

    /* Is RAID1 volume? */
    volumeNumber = mirrorRaidVolumeNameForPartitions.indexOf(volumeName);
    if (volumeNumber != -1)
    {
        return true;
    }

    /* Is RAID5 disk-3 volume? */
    volumeNumber = raid5with3DiskVolNameForPartitions.indexOf(volumeName);
    if (volumeNumber != -1)
    {
        return true;
    }

    /* Is RAID5 disk-4 volume? */
    volumeNumber = raid5with4DiskVolNameForPartitions.indexOf(volumeName);
    if (volumeNumber != -1)
    {
        return true;
    }

    /* Is RAID10 volume? */
    volumeNumber = raid10VolNameForPartitions.indexOf(volumeName);
    if (volumeNumber != -1)
    {
        return true;
    }

    /* Volume not found */
    return false;
}

void HDDGroup::getLogicalVolumeStatus()
{
    DevCommParam* param = new DevCommParam();
    param->msgType = MSG_SET_CMD;
    param->cmdType = LOGICAL_VOLUME_STS;
    applController->processActivity(currDevName, DEVICE_COMM, param);
    WizardCommon::LoadProcessBar();
}

void HDDGroup::getConfig()
{
    createPayload(MSG_GET_CFG);
}

void HDDGroup::saveConfig()
{
    if(volListSize == 0 || false == isUserChangeConfig())
    {
        m_deletePage = true;
        getConfig();
        return;
    }

    infoPage->loadInfoPage(ValidationMessage::getValidationMessage(HDD_GROUP_MANAGE_REC_STOP), true, false, "", CONFORMATION_BTN_YES, CONFORMATION_BTN_NO);
}

void HDDGroup::createPayload(REQ_MSG_ID_e msgType)
{
    QString payloadString = payloadLib->createDevCnfgPayload(msgType,
                                                             STORAGE_ALLOCATION_TABLE_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_FRM_INDEX,
                                                             CNFG_FRM_FIELD,
                                                             MAX_STORAGE_ALLOCATION_CFG_FIELD,
                                                             MAX_STORAGE_ALLOCATION_CFG_FIELD);
    DevCommParam* param = new DevCommParam();
    param->msgType = msgType;
    param->payload = payloadString;
    applController->processActivity(currDevName, DEVICE_COMM, param);
    WizardCommon::LoadProcessBar();
}

void HDDGroup::processDeviceResponse(DevCommParam *param, QString deviceName)
{
    WizardCommon::UnloadProcessBar();
    if (deviceName != currDevName)
    {
        return;
    }

    if (param->deviceStatus != CMD_SUCCESS)
    {
        EPRINT(CONFIG_PAGES,"[HDD Group]: [DEVICE_REPLY: %d]", param->deviceStatus);
        infoPage->loadInfoPage(ValidationMessage::getDeviceResponceMessage(param->deviceStatus));
        return;
    }

    switch(param->msgType)
    {
        case MSG_SET_CMD:
        {
            payloadLib->parseDevCmdReply(true, param->payload);
            switch(param->cmdType)
            {
                case LOGICAL_VOLUME_STS:
                {
                    quint8  volIdx, volStatus, volListIdx = 0;
                    QString volumeName;
                    volumeList.clear();

                    for(volIdx = 0; volIdx < MAX_LOGICAL_VOLUME_POSSIBLE; volIdx++)
                    {
                        volumeName = payloadLib->getCnfgArrayAtIndex(volIdx * MAX_LOG_DRV_FEILDS + LOG_VOL_NAME).toString();
                        volStatus = payloadLib->getCnfgArrayAtIndex(volIdx * MAX_LOG_DRV_FEILDS + LOG_STATUS).toUInt();

                        if ((volumeName == "") || (-1 != networkdriveList.indexOf(volumeName))
                                || (!((volStatus == LOG_STS_NORMAL) || (volStatus == LOG_STS_FULL) || (volStatus == LOG_STS_FORMATING))))
                        {
                            continue;
                        }
                        volumeList.insert(volListIdx++, volumeName);
                    }

                    volListSize = volumeList.size();
                    if(volListSize == 0)
                    {
                        showHideCtrls(false);
                    }
                    else
                    {
                        createVolumeList();
                        showHideCtrls(true);
                        updatePage(false);
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
            payloadLib->parsePayload(param->msgType, param->payload);

            if(payloadLib->getcnfgTableIndex() != STORAGE_ALLOCATION_TABLE_INDEX)
            {
                break;
            }

            m_configResponse.clear();
            for(quint8 grpIdx = 0; grpIdx < STORAGE_ALLOCATION_GROUP_MAX; grpIdx++)
            {
                storageAllocationInfo[grpIdx].volumeAllocationMask = payloadLib->getCnfgArrayAtIndex(STORAGE_ALLOCATION_VOLUME_MASK_GROUP_START + grpIdx).toUInt();
                m_configResponse[STORAGE_ALLOCATION_VOLUME_MASK_GROUP_START + grpIdx] = storageAllocationInfo[grpIdx].volumeAllocationMask;

                for(quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
                {
                    storageAllocationInfo[grpIdx].cameraAllocationMask.bitMask[maskIdx] = payloadLib->getCnfgArrayAtIndex
                            (STORAGE_ALLOCATION_CAMERA_MASK_GROUP_START + (grpIdx * CAMERA_MASK_MAX) + maskIdx).toULongLong();
                    m_configResponse[STORAGE_ALLOCATION_CAMERA_MASK_GROUP_START + (grpIdx * CAMERA_MASK_MAX) + maskIdx] = storageAllocationInfo[grpIdx].cameraAllocationMask.bitMask[maskIdx];
                }
            }

            getLogicalVolumeStatus();
        }
        break;

        case MSG_SET_CFG:
        {
            DPRINT(CONFIG_PAGES,"[HDD Group]: [SAVE_SUCCESS]");
            MessageBanner::addMessageInBanner(ValidationMessage::getValidationMessage(SUCCESS_SAVE_MSG));
            m_deletePage = true;
            getConfig();
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void HDDGroup::updateStorageAllocationMask()
{
    quint8 groupIdx = currentGroupSelected;
    qint8 volIndex = 0;

    for(quint8 index = 0; index < volListSize; index++)
    {
        if (false == getVolumeNumber(STRING_BEFORE_GRP(m_volumeListLabel[index]->getText()), volIndex))
        {
            continue;
        }

        if(isVolumeSelected[index])
        {
            SET_VOLUME_MASK_BIT(storageAllocationInfo[groupIdx].volumeAllocationMask, volIndex);
        }
        else
        {
            CLR_VOLUME_MASK_BIT(storageAllocationInfo[groupIdx].volumeAllocationMask, volIndex);
        }
    }

    for(quint8 index = 0; index < totalCamera; index++)
    {
        if(isCameraSelected[index])
        {
            SET_CAMERA_MASK_BIT(storageAllocationInfo[groupIdx].cameraAllocationMask, index);
        }
        else
        {
            CLR_CAMERA_MASK_BIT(storageAllocationInfo[groupIdx].cameraAllocationMask, index);
        }
    }

    DPRINT(CONFIG_PAGES, "[HDD Group]: [HDD GroupIdx=%d], [volumeMask=0x%x], [cameraMask1=0x%llx], [cameraMask2=0x%llx]", groupIdx,
            (storageAllocationInfo[groupIdx].volumeAllocationMask & UINT16_MAX), storageAllocationInfo[groupIdx].cameraAllocationMask.bitMask[0],
            (storageAllocationInfo[groupIdx].cameraAllocationMask.bitMask[1] & UINT32_MAX));
}

void HDDGroup::showVolumeList()
{
    qint8 volIndex;
    for(quint8 index = 0 ; index < volListSize; index++)
    {
        if (false == getVolumeNumber(STRING_BEFORE_GRP(m_volumeListLabel[index]->getText()), volIndex))
        {
            continue;
        }

        m_volumeListLabel[index]->changeText(volumeList.value(index));
        for(quint8 grpIdx = 0 ; grpIdx < STORAGE_ALLOCATION_GROUP_MAX; grpIdx++)
        {
            if (GET_VOLUME_MASK_BIT(storageAllocationInfo[grpIdx].volumeAllocationMask, volIndex))
            {
                m_volumeListLabel[index]->changeText(volumeList.value(index) + groupLableStrings[grpIdx]);
            }
        }
    }
}

void HDDGroup::showCameraList()
{
    quint8 fieldsOnPage;

    if(totalCamera < (MAX_CAM_ON_SINGLE_PAGE*(currentPageNum + 1)))
    {
        fieldsOnPage = totalCamera - ((MAX_CAM_ON_SINGLE_PAGE*(currentPageNum)));
    }
    else
    {
        fieldsOnPage = MAX_CAM_ON_SINGLE_PAGE;
    }

    for(quint8 index = 0; index < fieldsOnPage; index++)
    {
        m_cameraListLabel[index]->changeText(cameraList.value(index + MAX_CAM_ON_SINGLE_PAGE * currentPageNum));
        for(quint8 grpIdx = 0; grpIdx < STORAGE_ALLOCATION_GROUP_MAX; grpIdx++)
        {
            if (GET_CAMERA_MASK_BIT(storageAllocationInfo[grpIdx].cameraAllocationMask, index + MAX_CAM_ON_SINGLE_PAGE * currentPageNum))
            {
                m_cameraListLabel[index]->changeText(cameraList.value(index + MAX_CAM_ON_SINGLE_PAGE * currentPageNum) + groupLableStrings[grpIdx]);
            }
        }

        m_cameraListCheckBox[index]->changeState(isCameraSelected[index + MAX_CAM_ON_SINGLE_PAGE * currentPageNum] == true ? ON_STATE : OFF_STATE);
        m_cameraListLabel[index]->update();
        if(m_cameraListCheckBox[index]->isHidden())
        {
            m_cameraListCheckBox[index]->setVisible(true);
            m_cameraListCheckBox[index]->setIsEnabled(true);
        }
    }

    for(quint8 index = fieldsOnPage ; index < MAX_CAM_ON_SINGLE_PAGE; index++)
    {
        m_cameraListLabel[index]->changeText ("");
        m_cameraListLabel[index]->update();
        m_cameraListCheckBox[index]->setVisible(false);
        m_cameraListCheckBox[index]->setIsEnabled(false);
    }

    if(currentPageNum != 0)
    {
        m_cameraPrevButton->setIsEnabled (true);
    }
    else
    {
        if((currentElement == HDD_GROUP_CAMERA_NEXT_BTN) || (currentElement == HDD_GROUP_CAMERA_PREV_BTN))
        {
            currentElement = HDD_GROUP_CAMERA_NEXT_BTN;
        }
        m_cameraPrevButton->setIsEnabled (false);
    }

    if(currentPageNum == (totalPages-1))
    {
        if((currentElement == HDD_GROUP_CAMERA_NEXT_BTN) || (currentElement == HDD_GROUP_CAMERA_PREV_BTN))
        {
            currentElement = HDD_GROUP_CAMERA_PREV_BTN;
        }
        m_cameraNextButton->setIsEnabled (false);
    }
    else
    {
        m_cameraNextButton->setIsEnabled (true);
    }

    m_cameraPrevButton->resetGeometry(m_cameraSelectHeading->x(), (m_cameraListCheckBox[0]->y() + SCALE_HEIGHT(fieldsOnPage * 40)), m_cameraPrevButton->width(), m_cameraPrevButton->height());
    m_cameraNextButton->resetGeometry(m_cameraNextButton->x(), m_cameraPrevButton->y(), m_cameraNextButton->width(), m_cameraNextButton->height());

    for(quint8 index = 0; index < MAX_PAGE_NUMBER; index++)
    {
        /* Calculate pagelable index (Possible values: 0, 1, 2, 3) */
        quint8 tempIndex = currentPageNum % MAX_PAGE_NUMBER;

        /* Derive first page index from currently visible page numbers (Possible values: 0, 4, 8, 12, ...) */
        quint8 pageOffset = currentPageNum - (currentPageNum % MAX_PAGE_NUMBER);

        if(nextPageSelected == true)
        {
            /* Check if we need to change page number text */
            if((pageOffset + index) < totalPages)
            {
                m_cameraPageNumberLabel[index]->changeText (QString(" ") + QString("%1").arg(pageOffset + 1 + index) + QString(" "));
            }
            else
            {
                m_cameraPageNumberLabel[index]->changeText ("");
            }
        }
        else
        {
           /* Check if we need to change page number text */
           if(((currentPageNum + 1) % MAX_PAGE_NUMBER) == 0)
           {
               m_cameraPageNumberLabel[index]->changeText (QString(" ") + QString("%1").arg ((currentPageNum + 1) - (MAX_PAGE_NUMBER - index) + 1) + QString(" "));
           }
        }

        if((index == tempIndex) && ((pageOffset + index) < totalPages))
        {
            m_cameraPageNumberLabel[index]->setBackGroundColor (CLICKED_BKG_COLOR);
            m_cameraPageNumberLabel[index]->changeTextColor (HIGHLITED_FONT_COLOR);
            m_cameraPageNumberLabel[index]->setBold (true);
            m_cameraPageNumberLabel[index]->forceActiveFocus ();
            currentElement = (HDD_GROUP_CAMERA_PAGE_NUM_BTN + index);
        }
        else
        {
            m_cameraPageNumberLabel[index]->setBackGroundColor (TRANSPARENTKEY_COLOR);
            m_cameraPageNumberLabel[index]->changeTextColor (NORMAL_FONT_COLOR);
            m_cameraPageNumberLabel[index]->setBold (false);
            m_cameraPageNumberLabel[index]->deSelectControl ();
        }

        m_cameraPageNumberLabel[index]->setOffset(m_cameraPageNumberLabel[index]->x(), (m_cameraListCheckBox[0]->y() + SCALE_HEIGHT(fieldsOnPage * 40) + SCALE_HEIGHT(5)));
        m_cameraPageNumberLabel[index]->update();
    }
}

void HDDGroup::updatePage(bool isDropDownChng)
{
    qint8 volIndex = 0;
    quint8 totalVolumeSelected = 0;
    for(quint8 index = 0 ; index < volListSize; index++)
    {
        if (false == getVolumeNumber(STRING_BEFORE_GRP(m_volumeListLabel[index]->getText()), volIndex))
        {
            continue;
        }

        if (true == GET_VOLUME_MASK_BIT(storageAllocationInfo[currentGroupSelected].volumeAllocationMask, volIndex))
        {
            m_volumeListCheckBox[index]->changeState(ON_STATE);
            isVolumeSelected[index] = true;
            totalVolumeSelected++;
        }
        else
        {
            m_volumeListCheckBox[index]->changeState(OFF_STATE);
            isVolumeSelected[index] = false;
        }
    }

    m_allVolumeChecklist->changeState((totalVolumeSelected == volListSize) ? ON_STATE : OFF_STATE);

    if (isDropDownChng == true)
    {
        currentPageNum = 0;
    }

    totalCameraSelected = 0;
    for(quint8 index = 0 ; index < totalCamera; index++)
    {
        if (true == GET_CAMERA_MASK_BIT(storageAllocationInfo[currentGroupSelected].cameraAllocationMask, index))
        {
            isCameraSelected[index] = true;
            totalCameraSelected++;
        }
        else
        {
            isCameraSelected[index] = false;
        }
    }

    m_allCameraChecklist->changeState((totalCameraSelected == totalCamera) ? ON_STATE : OFF_STATE);

    showCameraList();
    showVolumeList();
}

bool HDDGroup::isCameraAllocatedToAnyOtherGrp(quint8 camIdx)
{
    for(quint8 grpIdx = 0; grpIdx < STORAGE_ALLOCATION_GROUP_MAX; grpIdx++)
    {
        if (grpIdx == currentGroupSelected)
        {
            continue;
        }

        if(isAllChkBoxClicked == false)
        {
            if (GET_CAMERA_MASK_BIT(storageAllocationInfo[grpIdx].cameraAllocationMask, camIdx))
            {
                tmpIdx = camIdx - (MAX_CAM_ON_SINGLE_PAGE * currentPageNum) + HDD_GROUP_SEL_ALL_CAMERA + 1;
                tmpGrpIdx = grpIdx;
                return true;
            }
        }
        else
        {
            if (!(IS_ALL_CAMERA_MASK_BIT_CLR(storageAllocationInfo[grpIdx].cameraAllocationMask)))
            {
                return true;
            }
        }
    }

    return false;
}

bool HDDGroup::isVolumeAllocatedToAnyGrp(quint8 volIdx)
{
    qint8 tVolIndex = 0;
    for(quint8 grpIdx = 0; grpIdx < STORAGE_ALLOCATION_GROUP_MAX; grpIdx++)
    {
        if (grpIdx == currentGroupSelected)
        {
            continue;
        }

        if (isAllChkBoxClicked == false)
        {
            if (false == getVolumeNumber(STRING_BEFORE_GRP(m_volumeListLabel[volIdx]->getText()), tVolIndex))
            {
                continue;
            }

            if (GET_VOLUME_MASK_BIT(storageAllocationInfo[grpIdx].volumeAllocationMask, tVolIndex))
            {
                tmpIdx = volIdx + HDD_GROUP_VOLUME_LIST;
                tmpGrpIdx = grpIdx;
                return true;
            }
        }
        else
        {
            if (storageAllocationInfo[grpIdx].volumeAllocationMask != 0)
            {
                return true;
            }
        }
    }

    return false;
}

void HDDGroup::slotInfoPageBtnclick(int index)
{
    if (index != INFO_OK_BTN)
    {
        if (infoPage->getText() == ValidationMessage::getValidationMessage(HDD_GROUP_MANAGE_REC_STOP))
        {
            m_deletePage = true;
            getConfig();
        }

        isAllChkBoxClicked = false;
        return;
    }

    if (infoPage->getText() == ValidationMessage::getValidationMessage(HDD_GROUP_MANAGE_DISK_ALREADY_ASSIGN))
    {
        if(isAllChkBoxClicked == true)
        {
            for(quint8 index = 0; index < STORAGE_ALLOCATION_GROUP_MAX; index++)
            {
                storageAllocationInfo[index].volumeAllocationMask = 0;
            }

            slotOptionButtonClicked(ON_STATE, HDD_GROUP_SEL_ALL_VOLUME);
            return;
        }

        qint8 volIndex = 0;
        if (false == getVolumeNumber(STRING_BEFORE_GRP(m_volumeListLabel[tmpIdx - HDD_GROUP_VOLUME_LIST]->getText()), volIndex))
        {
            return;
        }

        CLR_VOLUME_MASK_BIT(storageAllocationInfo[tmpGrpIdx].volumeAllocationMask, volIndex);
        m_volumeListCheckBox[tmpIdx  - HDD_GROUP_VOLUME_LIST]->changeState(ON_STATE);
        slotOptionButtonClicked(ON_STATE, tmpIdx);
        return;
    }

    if (infoPage->getText() == ValidationMessage::getValidationMessage(HDD_GROUP_MANAGE_CAMERA_ALREADY_ASSIGN))
    {
        if(isAllChkBoxClicked == true)
        {
            for(quint8 index = 0; index < STORAGE_ALLOCATION_GROUP_MAX; index++)
            {
                memset(&storageAllocationInfo[index].cameraAllocationMask, 0, sizeof(CAMERA_BIT_MASK_t));
            }

            slotOptionButtonClicked(ON_STATE, HDD_GROUP_SEL_ALL_CAMERA);
            return;
        }

        CLR_CAMERA_MASK_BIT(storageAllocationInfo[tmpGrpIdx].cameraAllocationMask, (tmpIdx + (MAX_CAM_ON_SINGLE_PAGE * currentPageNum) - HDD_GROUP_SEL_ALL_CAMERA - 1));
        m_cameraListCheckBox[((tmpIdx + (MAX_CAM_ON_SINGLE_PAGE * currentPageNum) - HDD_GROUP_SEL_ALL_CAMERA - 1) % MAX_CAM_ON_SINGLE_PAGE)]->changeState(ON_STATE);
        slotOptionButtonClicked(ON_STATE, tmpIdx);
        return;
    }

    if (infoPage->getText() == ValidationMessage::getValidationMessage(HDD_GROUP_MANAGE_REC_STOP))
    {
        for(quint8 grpIdx = 0; grpIdx < STORAGE_ALLOCATION_GROUP_MAX; grpIdx++)
        {
            payloadLib->setCnfgArrayAtIndex((STORAGE_ALLOCATION_VOLUME_MASK_GROUP_START + grpIdx), storageAllocationInfo[grpIdx].volumeAllocationMask);

            for(quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
            {
                payloadLib->setCnfgArrayAtIndex((STORAGE_ALLOCATION_CAMERA_MASK_GROUP_START + (grpIdx * CAMERA_MASK_MAX) + maskIdx),
                                                storageAllocationInfo[grpIdx].cameraAllocationMask.bitMask[maskIdx]);
            }
        }

        createPayload(MSG_SET_CFG);
    }
}

void HDDGroup::slotDropdownValueChanged(QString idxStr, quint32 index)
{
    if(index != HDD_GROUP_DROP_DOWN)
    {
        return;
    }

    currentGroupSelected = hddGroupList.key(idxStr);
    updatePage(true);
}

void HDDGroup::slotButtonClick(int index)
{
    switch(index)
    {
        case HDD_GROUP_CAMERA_NEXT_BTN:
        {
            if (currentPageNum != (totalPages - 1))
            {
                currentPageNum++;
            }

            nextPageSelected = true;
            showCameraList();
        }
        break;

        case HDD_GROUP_CAMERA_PREV_BTN:
        {
            if(currentPageNum > 0)
            {
                currentPageNum --;
            }
            nextPageSelected = false;
            showCameraList();
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}

void HDDGroup::slotOptionButtonClicked(OPTION_STATE_TYPE_e state, int index)
{
    if(index == HDD_GROUP_SEL_ALL_VOLUME)
    {
        isAllChkBoxClicked = true;
        if((state == ON_STATE) && (isVolumeAllocatedToAnyGrp(volListSize) == true))
        {
            m_allVolumeChecklist->changeState(OFF_STATE);
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(HDD_GROUP_MANAGE_DISK_ALREADY_ASSIGN), true, false, "", CONFORMATION_BTN_YES, CONFORMATION_BTN_NO);
            return;
        }

        m_allVolumeChecklist->changeState(state);

        for(quint8 index = 0; index < volListSize; index++)
        {
            m_volumeListCheckBox[index]->changeState(state);
        }

        for (quint8 index = 0 ; index < volListSize ; index++)
        {
            isVolumeSelected[index] = (state == ON_STATE ? true : false);
        }

        isAllChkBoxClicked = false;
    }
    else if((index >= HDD_GROUP_VOLUME_LIST) && (index < HDD_GROUP_SEL_ALL_CAMERA))
    {
        if((state == ON_STATE) && (isVolumeAllocatedToAnyGrp((index - HDD_GROUP_VOLUME_LIST)) == true))
        {
            m_volumeListCheckBox[index - HDD_GROUP_VOLUME_LIST]->changeState(OFF_STATE);
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(HDD_GROUP_MANAGE_DISK_ALREADY_ASSIGN), true, false, "", CONFORMATION_BTN_YES, CONFORMATION_BTN_NO);
            return;
        }

        isVolumeSelected[index - HDD_GROUP_SEL_ALL_VOLUME - 1] = (state == ON_STATE ? true : false);
        quint8 selectedVolumes = 0;
        for(quint8 index = 0; index < volListSize; index++)
        {
            if(isVolumeSelected[index] == true)
            {
                selectedVolumes++;
            }
        }

        m_allVolumeChecklist->changeState((selectedVolumes == volListSize) ? ON_STATE : OFF_STATE);
    }
    else if (index == HDD_GROUP_SEL_ALL_CAMERA)
    {
        isAllChkBoxClicked = true;
        if((state == ON_STATE) && (isCameraAllocatedToAnyOtherGrp(totalCamera) == true))
        {
            m_allCameraChecklist->changeState(OFF_STATE);
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(HDD_GROUP_MANAGE_CAMERA_ALREADY_ASSIGN), true, false, "", CONFORMATION_BTN_YES, CONFORMATION_BTN_NO);
            return;
        }

        m_allCameraChecklist->changeState(state);

        for(quint8 index = 0; index < MAX_CAM_ON_SINGLE_PAGE; index++)
        {
            m_cameraListCheckBox[index]->changeState(state);
        }

        for (quint8 index = 0 ; index < totalCamera ; index++)
        {
            isCameraSelected[index] = (state == ON_STATE ? true : false);
        }

        isAllChkBoxClicked = false;
    }
    else if ((index >= HDD_GROUP_CAMERA_LIST) && (index < HDD_GROUP_CAMERA_PREV_BTN))
    {
        if((state == ON_STATE) && (isCameraAllocatedToAnyOtherGrp((index + (MAX_CAM_ON_SINGLE_PAGE * currentPageNum) - HDD_GROUP_SEL_ALL_CAMERA - 1)) == true))
        {
            m_cameraListCheckBox[index - HDD_GROUP_CAMERA_LIST]->changeState(OFF_STATE);
            infoPage->loadInfoPage(ValidationMessage::getValidationMessage(HDD_GROUP_MANAGE_CAMERA_ALREADY_ASSIGN), true, false, "", CONFORMATION_BTN_YES, CONFORMATION_BTN_NO);
            return;
        }

        isCameraSelected[index + (MAX_CAM_ON_SINGLE_PAGE * currentPageNum) - HDD_GROUP_SEL_ALL_CAMERA - 1] = (state == ON_STATE ? true : false);
        quint8 selectedCameras = 0;

        for(quint8 index = 0; index < totalCamera; index++)
        {
            if(isCameraSelected[index] == true)
            {
                selectedCameras++;
            }
        }

        m_allCameraChecklist->changeState((selectedCameras == totalCamera) ? ON_STATE : OFF_STATE);
    }

    updateStorageAllocationMask();
    showVolumeList();
    showCameraList();
}

void HDDGroup::slotPageNumberButtonClick(QString str)
{
    currentPageNum = ((quint8)str.toUInt() - 1);
    showCameraList();
}

void HDDGroup::slotUpdateCurrentElement(int index)
{
    currentElement = index;
}

bool HDDGroup::isUserChangeConfig()
{
    if(m_configResponse.isEmpty())
    {
        return false;
    }

    for(quint8 grpIdx = 0; grpIdx < STORAGE_ALLOCATION_GROUP_MAX; grpIdx++)
    {
        if (m_configResponse[STORAGE_ALLOCATION_VOLUME_MASK_GROUP_START + grpIdx] != storageAllocationInfo[grpIdx].volumeAllocationMask)
        {
           return true;
        }

        for(quint8 maskIdx = 0; maskIdx < CAMERA_MASK_MAX; maskIdx++)
        {
            if (m_configResponse[STORAGE_ALLOCATION_CAMERA_MASK_GROUP_START + (grpIdx * CAMERA_MASK_MAX) + maskIdx] !=
                    storageAllocationInfo[grpIdx].cameraAllocationMask.bitMask[maskIdx])
            {
                return true;
            }
        }
    }

    return false;
}
