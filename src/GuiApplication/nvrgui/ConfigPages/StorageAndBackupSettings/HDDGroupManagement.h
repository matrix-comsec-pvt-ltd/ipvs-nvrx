#ifndef HDDGROUPMANAGEMENT_H
#define HDDGROUPMANAGEMENT_H

#include "Elidedlabel.h"
#include "Controls/ConfigPageControl.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/ElementHeading.h"
#include "Controls/ControlButton.h"
#include "Controls/TextWithBackground.h"
#include "Controls/DropDown.h"
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

} STORAGE_ALLOCATION_INFO_t;

class HDDGroupManagement : public ConfigPageControl
{
    Q_OBJECT
public:

    explicit HDDGroupManagement(QString deviceName, QWidget *parent = 0, DEV_TABLE_INFO_t *devTabInfo = NULL);

    ~HDDGroupManagement();

    void getConfig();
    void saveConfig();
    void defaultConfig();
    void handleInfoPageMessage(int);
    void processDeviceResponse(DevCommParam *param, QString deviceName);

signals:

public slots:

    void slotDropdownValueChanged(QString, quint32);
    void slotButtonClick(int index);
    void slotOptionButtonClicked(OPTION_STATE_TYPE_e state, int index);
    void slotPageNumberButtonClick(QString str);

private:

    // private Variables
    bool                        isVolumeSelected[MAX_LOGICAL_VOLUME_POSSIBLE];
    bool                        isCameraSelected[MAX_CAMERAS];
    bool                        isAllChkBoxClicked;
    bool                        nextPageSelected;

    QMap<quint8, QString>       hddGroupList;
    QMap<quint8, QString>       volumeList;
    QMap<quint8, QString>       cameraList;

    quint8                      totalCamera;
    quint8                      totalPages;
    quint8                      currentGroupSelected;
    quint8                      currentPageNum;
    quint8                      volListSize;
    quint8                      totalCameraSelected;
    quint8                      tmpIdx;
    quint8                      tmpGrpIdx;

    STORAGE_ALLOCATION_INFO_t   storageAllocationInfo[STORAGE_ALLOCATION_GROUP_MAX];

    ElementHeading*             m_hddGroupHeading;
    DropDown*                   m_hddGroupDropDown;

    TextLabel*                  m_volumeListLabel[MAX_LOGICAL_VOLUME_POSSIBLE];
    ElementHeading*             m_volumeSelectHeading;
    OptionSelectButton*         m_allVolumeChecklist;
    OptionSelectButton*         m_volumeListCheckBox[MAX_LOGICAL_VOLUME_POSSIBLE];

    TextLabel*                  m_cameraListLabel[MAX_CAM_ON_SINGLE_PAGE];
    ElementHeading*             m_cameraSelectHeading;
    OptionSelectButton*         m_allCameraChecklist;
    OptionSelectButton*         m_cameraListCheckBox[MAX_CAM_ON_SINGLE_PAGE];

    ControlButton*              m_cameraNextButton;
    ControlButton*              m_cameraPrevButton;

    TextWithBackground*         m_cameraPageNumberLabel[MAX_PAGE_NUMBER];

    ElementHeading*             m_elementHeadingBitRate;
    ElidedLabel*                m_footnoteLabelBitRate;

    ElementHeading*             m_elementHeadingNoDevice;
    ElidedLabel*                m_footnoteLabelNoDevice;

    // private Function
    void createDefaultComponents();
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
    bool isUserChangeConfig();
};
#endif // HDDGROUPMANAGEMENT_H
