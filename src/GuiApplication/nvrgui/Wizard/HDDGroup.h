#ifndef HDDGROUP_H
#define HDDGROUP_H

#include "Elidedlabel.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/ElementHeading.h"
#include "Controls/ControlButton.h"
#include "Controls/TextWithBackground.h"
#include "Controls/DropDown.h"
#include "ValidationMessage.h"
#include "Controls/InfoPage.h"
#include "Controls/MessageBanner.h"
#include "PayloadLib.h"
#include "WizardCommon.h"
#include "stdint.h"

#define MAX_CAM_ON_SINGLE_PAGE          8
#define MAX_PAGE_NUMBER                 4
#define STORAGE_ALLOCATION_GROUP_MAX    8

/* Below Macro is valid only for RK3588 models */
#define MAX_LOGICAL_VOLUME_POSSIBLE     8

typedef struct
{
    UINT32              volumeAllocationMask;       /* Storage volume allocation to storage group */
    CAMERA_BIT_MASK_t   cameraAllocationMask;       /* Camera allocation to storage group */

} WIZ_STORAGE_ALLOCATION_INFO_t;

class HDDGroup : public WizardCommon
{
    Q_OBJECT
public:

    explicit HDDGroup(QString deviceName, QString subHeadStr, QWidget *parent = 0, WIZARD_PAGE_INDEXES_e pageId = MAX_WIZ_PG);

    ~HDDGroup();

    void getConfig();
    void saveConfig();
    void processDeviceResponse(DevCommParam *param, QString deviceName);

signals:

public slots:

    void slotDropdownValueChanged(QString, quint32);
    void slotButtonClick(int index);
    void slotOptionButtonClicked(OPTION_STATE_TYPE_e state, int index);
    void slotPageNumberButtonClick(QString str);
    void slotInfoPageBtnclick(int);
    void slotUpdateCurrentElement(int);
    bool isUserChangeConfig();

private:

    // private Variables
    bool                            isVolumeSelected[MAX_LOGICAL_VOLUME_POSSIBLE];
    bool                            isCameraSelected[MAX_CAMERAS];
    bool                            isAllChkBoxClicked;
    bool                            nextPageSelected;

    int                             currentElement;

    QMap<quint8, QString>           hddGroupList;
    QMap<quint8, QString>           volumeList;
    QMap<quint8, QString>           cameraList;
    QMap<quint32,QVariant>          m_configResponse;

    quint8                          totalCamera;
    quint8                          totalPages;
    quint8                          currentGroupSelected;
    quint8                          currentPageNum;
    quint8                          volListSize;
    quint8                          totalCameraSelected;
    quint8                          tmpIdx;
    quint8                          tmpGrpIdx;

    QString                         currDevName;

    WIZ_STORAGE_ALLOCATION_INFO_t   storageAllocationInfo[STORAGE_ALLOCATION_GROUP_MAX];
    DEV_TABLE_INFO_t                devTableInfo;

    ApplController*                 applController;
    PayloadLib*                     payloadLib;
    InfoPage*                       infoPage;
    TextLabel*                      m_hddGroupHeading;
    ElementHeading*                 m_hddGroupDropDownLabel;
    DropDown*                       m_hddGroupDropDown;

    TextLabel*                      m_volumeListLabel[MAX_LOGICAL_VOLUME_POSSIBLE];
    ElementHeading*                 m_volumeSelectHeading;
    OptionSelectButton*             m_allVolumeChecklist;
    OptionSelectButton*             m_volumeListCheckBox[MAX_LOGICAL_VOLUME_POSSIBLE];

    TextLabel*                      m_cameraListLabel[MAX_CAM_ON_SINGLE_PAGE];
    ElementHeading*                 m_cameraSelectHeading;
    OptionSelectButton*             m_allCameraChecklist;
    OptionSelectButton*             m_cameraListCheckBox[MAX_CAM_ON_SINGLE_PAGE];

    ControlButton*                  m_cameraNextButton;
    ControlButton*                  m_cameraPrevButton;

    TextWithBackground*             m_cameraPageNumberLabel[MAX_PAGE_NUMBER];

    ElementHeading*                 m_elementHeadingBitRate;
    ElidedLabel*                    m_footnoteLabelBitRate;

    ElementHeading*                 m_elementHeadingNoDevice;
    ElidedLabel*                    m_footnoteLabelNoDevice;

    // private Function
    void createDefaultComponents(QString);
    void createVolumeList();
    void destroyVolumeList();
    void showHideCtrls(bool);
    void createPayload(REQ_MSG_ID_e);
    void fillCameraList();
    bool getVolumeNumber(QString, qint8&);
    void getLogicalVolumeStatus();
    void updateStorageAllocationMask();
    void updatePage(bool isDropDownChng);
    void showVolumeList();
    void showCameraList();
    bool isCameraAllocatedToAnyOtherGrp(quint8);
    bool isVolumeAllocatedToAnyGrp(quint8);
};

#endif // HDDGROUP_H
