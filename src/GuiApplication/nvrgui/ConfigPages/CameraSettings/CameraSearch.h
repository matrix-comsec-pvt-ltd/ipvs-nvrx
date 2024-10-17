#ifndef CAMERASEARCH_H
#define CAMERASEARCH_H

#include <QWidget>
#include <QFrame>
#include "Elidedlabel.h"
#include "Controls/ConfigPageControl.h"
#include "Controls/TableCell.h"
#include "Controls/TextLabel.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/ControlButton.h"
#include "Controls/CnfgButton.h"
#include "Controls/CameraSearchProcess.h"
#include "Controls/PageOpenButton.h"
#include "Controls/DropDown.h"
#include "Controls/TextLabel.h"
#include "ConfigPages/CameraSettings/AddCamera.h"
#include "ConfigPages/CameraSettings/TestCamera.h"
#include "ConfigPages/CameraSettings/AdvanceCameraSearch.h"
#include "ConfigPages/CameraSettings/CamSearchFailReport.h"
#include "ConfigField.h"
#include "ConfigPages/BasicSettings/AutoConfigureCamera.h"
#include "AutoAddCameraList.h"

#define MAX_RECORD_DATA         7 
#define MAX_RECORD_FEILDS       8

class CameraSearchParam
{
public:
    MX_CAM_STATUS_e camStatus;
    QString         ipv4Address;
    QString         ipv6Address; 
    QString         httpPort;
    QString         onvifPort;
    QString         brand;
    QString         model;
    bool            onvifSupportF;
    quint16         camNum;
    QString         camName;

    CameraSearchParam()
    {
        clear();
    }

    void clear(void)
    {
        camStatus = MAX_MX_CAM_STATUS;
        ipv4Address = "";
        ipv6Address = "";  
        httpPort = "";
        onvifPort = "";
        brand = "";
        model = "";
        onvifSupportF = false;
        camNum = 0;
        camName = "";
    }

    bool operator == (const CameraSearchParam & other) const
    {
        if ((ipv4Address != other.ipv4Address) || (ipv6Address != other.ipv6Address) || (onvifSupportF != other.onvifSupportF))
        {
            return false;
        }

        if (true == onvifSupportF)
        {
            if (onvifPort == other.onvifPort)
            {
                return true;
            }
        }
        else
        {
            if (httpPort == other.httpPort)
            {
                return true;
            }
        }

        return false;
    }
};

class CameraSearch : public ConfigPageControl
{
    Q_OBJECT
public:
    explicit CameraSearch(QString deviceName, QWidget* parent = 0, DEV_TABLE_INFO_t *devTabInfo = NULL);
    ~CameraSearch();

    void processDeviceResponse(DevCommParam *param, QString deviceName);
    void handleInfoPageMessage(int);
    void updateList(DevCommParam *param);
    void getConfig();

signals:

public slots:
    void slotButtonClick(int);
    void slotAddCameraDelete(quint8 listIndex, bool saveCameraFlag,
                             QString ipAddressString, QString httpPortStr,
                             QString onvifPortString, bool onvifSupportF,
                             QString brandListStr, QString modelListStr,
                             QString camName, QString userName, QString tPassword,
                             quint8 selIndex, quint8 currentIndex);

    void slotCreateCMDRequest(SET_COMMAND_e cmdType,quint8 totalFeilds);
    void slotTestCamDelete(quint8);
    void slotAdvanceSearchDelete(bool);
    void slotFailReportDelete();
    void slotFilterValueChanged(QString,quint32);
    void slotOptionSelectionButton(OPTION_STATE_TYPE_e,int);
    void slotAddButtonClick(int);
    void slotTestButtonClick(int);
    void slotautoAddIconTimeOut();
    void slotAdvanceSearchRange(QString,QString,QString);
    void slotGetAcqListTimerTimeout();
    void slotObjectDelete();

private:

    // private variables
    bool isUpdateBeforeSearch;
    bool isInfoPageLoaded;
    bool isCancelSend;
    bool isAdvanceCameraSearchRunning;
    bool isBlockListFlag;

    SEARCH_FILTER_TYPE_e        currentFilterType;
    quint8                      currentPageNo;
    quint8                      maximumPages;
    quint8                      maxSearchListCount;
    quint8                      testCameraIndex;

    QString                     username;
    QString                     password;
    CameraSearchParam           requestCameraParam;
    QString                     advCamIpAddr1;
    QString                     advCamIpAddr2;
    QString                     advCamHttpPort;

    QVector<CameraSearchParam>  cameraSearchList;
    QVector<CameraSearchParam>  cameraAddedList;
    QVector<CameraSearchParam>  cameraNotAddedList;
    QVector<CameraSearchParam>  autoConfigCameraList;

    QStringList                 failIpAddresslist;
    QStringList                 failReasonList;
    QTimer*                     autoAddIconTimer;

    // Controls
    ElidedLabel                 *elided[MAX_RECORD_FEILDS - 1];
    ElementHeading*             elementHeading[MAX_RECORD_FEILDS - 1];
    BgTile*                     topBgtile;
    BgTile*                     bottomBgtile;
    DropDown*                   searchTypeDropDown;

    OptionSelectButton*         selectAllCam;
    OptionSelectButton*         selectCam[MAX_RECORD_DATA];

    TableCell*                  fieldsHeading[MAX_RECORD_FEILDS];
    TableCell*                  srNumber[MAX_RECORD_DATA];
    TableCell*                  ipAddressCell[MAX_RECORD_DATA];
    TableCell*                  httpPorts[MAX_RECORD_DATA];
    TableCell*                  brands[MAX_RECORD_DATA];
    TableCell*                  model[MAX_RECORD_DATA];
    TableCell*                  onvifPorts[MAX_RECORD_DATA];
    TableCell*                  addCameraCell[MAX_RECORD_DATA];
    TableCell*                  testCamera[MAX_RECORD_DATA];

    TextLabel*                  ipv4AddressStr[MAX_RECORD_DATA];
    TextLabel*                  ipv6AddressStr[MAX_RECORD_DATA];
    TextLabel*                  httpPortsStr[MAX_RECORD_DATA];
    TextLabel*                  brandsStr[MAX_RECORD_DATA];
    TextLabel*                  modelStr[MAX_RECORD_DATA];
    TextLabel*                  onvifPortStr[MAX_RECORD_DATA];

    TextLabel*                  searchRunningInfo;

    ControlButton*              addCameraBtn[MAX_RECORD_DATA];
    ControlButton*              testCameraBtn[MAX_RECORD_DATA];

    ControlButton*              previousButton;
    ControlButton*              nextButton;

    CnfgButton*                 searchButton;
    CnfgButton*                 stopButton;
    CnfgButton*                 autoAddButton;

    PageOpenButton*             advnaceSearchButton;
    PageOpenButton*             failReportButton;

    PageOpenButton*             autoAddCameraButton;

    // objects
    AddCamera*                  addCamera;
    TestCamera*                 testCam;

    AdvanceCameraSearch*        advanceCameraSearch;
    CamSearchFailReport*        camSearchFailReport;
    AutoConfigureCamera*        autoConfigureCamera;
    QStringList                 autoConfigureStringList;

    CameraSearchProcess*        cameraSearchProcess;
    QTimer*                     getAcqListTimer;
    AutoAddCameraList*          m_autoAddCamera;

    // private Fuction
    void createDefaultComponent();
    void showCameraSearchList();
    void clearSerachDisplayList(quint8 index = 0);
    void updateNavigationControlStatus();
    void sendCamAutoSearchCmd();
    void cancelCamAutoSearchCmd();
    void sendCommand(SET_COMMAND_e cmdType, qint32 totalfeilds = 0);
    void fillFilterList();
    void clearAllList();

    void fillAutoConfigList();
    void sendAutoAddCmd();
    void updateSelCamState(bool isStateOn = false);
    void updateControlsforSearch(bool enableContrl);
    void loadInfoPageMsg(QString infoMsg);
    bool getCameraSearchListIndex(const CameraSearchParam &cameraParam, quint8 &listIndex);
    bool getAutoConfigCameraListIndex(const CameraSearchParam &cameraParam, quint8 &listIndex);
};

#endif // CAMERASEARCH_H
