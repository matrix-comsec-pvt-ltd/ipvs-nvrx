#ifndef SEARCHCAMERA_H
#define SEARCHCAMERA_H

#include <QWidget>
#include "WizardCommon.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/Heading.h"
#include "Controls/TableCell.h"
#include "Controls/ControlButton.h"
#include "Controls/CnfgButton.h"
#include "Controls/ElementHeading.h"
#include "ApplController.h"
#include "PayloadLib.h"
#include "EnumFile.h"
#include "Controls/InfoPage.h"
#include "WizardAddCamera.h"
#include "WizardTestCamera.h"
#include "ConfigField.h"
#include "Controls/CameraSearchProcess.h"

#define MAX_CAM_SEARCH_RECORD_DATA         7
#define MAX_CAM_SEARCH_FEILDS              8

typedef enum
{
    WIZ_FILTER_NONE,
    WIZ_FILTER_ADDED,
    WIZ_FILTER_NOT_ADDED
}WIZ_SEARCH_FILTER_TYPE_e;

class SearchCamera : public WizardCommon
{
    Q_OBJECT
public:
    explicit SearchCamera(QString devName, QString subHeadStr, QWidget *parent = 0, WIZARD_PAGE_INDEXES_e pageId = MAX_WIZ_PG);
    virtual ~SearchCamera();
    void createDefaultElements(QString subHeadStr);
    void processDeviceResponse (DevCommParam *param, QString deviceName);
    void updateList(DevCommParam *param);
    void getConfig();
    void saveConfig();

signals:

public slots:
    void slotButtonClick(int);
    void slotAddCameraDelete(quint8 cameraIndex, bool saveCameraFlag,
                             QString ipAddressString, QString httpPortStr,
                             QString tOnvifPortStr, bool tOnvifSupport,
                             QString brandlistStr, QString modellistStr,
                             QString camName,QString userName,QString tPassword,
                             quint8 selIndex,quint8 currentIndex);

    void slotCreateCMDRequest(SET_COMMAND_e cmdType,quint8 totalFeilds);
    void slotTestCamDelete(quint8);
    void slotOptionSelectionButton(OPTION_STATE_TYPE_e,int);
    void slotAddButtonClick(int);
    void slotTestButtonClick(int);
    void slotautoAddIconTimeOut();
    void slotInfoPageBtnclick(int);
    void slotGetAcqListTimerTimeout();

private:

    // private variables
    bool isUpdateBeforeSearch;
    bool isInfoPageLoaded;
    bool isCancelSend;
    bool isAdvanceCameraSearchRunning;

    DEVICE_REPLY_TYPE_e                 deviceReponseForFailReport;
    BOOL                                isBlockListFlag;
    QString                             currDevName;
    quint8                              currentPageNo;
    quint8                              maximumPages;
    quint8                              maxSearchListCount;
    quint8                              testCameraIndex;
    QString                             username;
    QString                             password;
    QString                             currentBrandName;
    QString                             currentModelName;
    QString                             currentCameraName;
    QString                             requestedIp;
    QString                             requestedPort;
    QStringList                         searchList;
    QStringList                         camStatusList;
    QStringList                         httpPortList;
    QStringList                         onvifPortList;
    QStringList                         brandList;
    QStringList                         modelList;
    QStringList                         onvifSupportStatusList;
    QStringList                         camNumList;
    QStringList                         camNameList;
    QList<QPair<QString, QString>>      autoAddIpList;
    QList<QPair<QString, QString>>      ipAddressList;
    QTimer*                             autoAddIconTimer;
    bool                                retainIpAddrInAutoConfig;
    qint32                              autoConfigIpAddrFamily;

    TextLabel*              m_cameraSearchHeading;
    ApplController*         applController;
    PayloadLib*             payloadLib;
    OptionSelectButton*     selectAllCam;
    OptionSelectButton*     selectCam[MAX_CAM_SEARCH_RECORD_DATA];
    TableCell*              fieldsHeading[MAX_CAM_SEARCH_FEILDS];
    TableCell*              srNumber[MAX_CAM_SEARCH_RECORD_DATA];
    TableCell*              ipAddressCell[MAX_CAM_SEARCH_RECORD_DATA];
    TableCell*              httpPorts[MAX_CAM_SEARCH_RECORD_DATA];
    TableCell*              brands[MAX_CAM_SEARCH_RECORD_DATA];
    TableCell*              model[MAX_CAM_SEARCH_RECORD_DATA];
    TableCell*              onvifPorts[MAX_CAM_SEARCH_RECORD_DATA];
    TableCell*              addCameraCell[MAX_CAM_SEARCH_RECORD_DATA];
    TableCell*              testCamera[MAX_CAM_SEARCH_RECORD_DATA];
    TextLabel*              fieldsHeadingStr[MAX_CAM_SEARCH_FEILDS - 1];
    TextLabel*              srNumberStr[MAX_CAM_SEARCH_RECORD_DATA];
    TextLabel*              ipv4AddressStr[MAX_CAM_SEARCH_RECORD_DATA];
    TextLabel*              ipv6AddressStr[MAX_CAM_SEARCH_RECORD_DATA];
    TextLabel*              httpPortsStr[MAX_CAM_SEARCH_RECORD_DATA];
    TextLabel*              brandsStr[MAX_CAM_SEARCH_RECORD_DATA];
    TextLabel*              modelStr[MAX_CAM_SEARCH_RECORD_DATA];
    TextLabel*              onvifPortStr[MAX_CAM_SEARCH_RECORD_DATA];
    TextLabel*              searchRunningInfo;
    ControlButton*          addCameraBtn[MAX_CAM_SEARCH_RECORD_DATA];
    ControlButton*          testCameraBtn[MAX_CAM_SEARCH_RECORD_DATA];
    ControlButton*          previousButton;
    ControlButton*          nextButton;
    CnfgButton*             addButton;
    WizardAddCamera*        addCamera;
    WizardTestCamera*       testCam;
    CameraSearchProcess*    cameraSearchProcess;
    QTimer*                 getAcqListTimer;
    DEV_TABLE_INFO_t        devTableInfo;
    InfoPage*               infoPage;
    bool                    loadInfoPageFlag;

    // private Fuction
    void showCameraSearchList();
    void clearSerachDisplayList(quint8 index = 0);
    void updateNavigationControlStatus();
    void sendCamAutoSearchCmd();
    void cancelCamAutoSearchCmd();
    void sendCommand(SET_COMMAND_e cmdType, quint32 totalfeilds = 0);
    void clearAllList();

    void fillAutoAddIpList();
    void sendAutoAddCmd();
    void updateSelCamState(bool isStateOn = false);
    void updateControlsforSearch(bool);
    void loadInfoPageMsg(QString infoMsg);

};
#endif // SEARCHCAMERA_H
