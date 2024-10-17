#ifndef CAMERALIST_H
#define CAMERALIST_H

#include <QWidget>
#include "ApplController.h"
#include "CameraListButton.h"
#include "DeviceListButton.h"
#include "TextLabel.h"
#include "ScrollBar.h"
#include "Controls/ToolTip.h"

#define MAX_CAMERA_TILE         (MAX_CAMERAS + 1)

//typedef enum
//{
//    CALLED_BY_DISPLAY_SETTING,
//    CALLED_BY_WINDOWSEQ_SETTING,
//    CALLED_BY_VIEWCAM_ADD_LIST,
//    MAX_CALLED_BY_CLASS
//} CAMERALIST_CALLED_BY_e;

class CameraList : public KeyBoard, public NavigationControl
{
    Q_OBJECT
private:
    DeviceListButton                                                *m_devices[MAX_DEVICES];
    CameraListButton                                                *m_cameras[MAX_DEVICES][MAX_CAMERA_TILE];
    ScrollBar                                                       *m_scrollbar;
    ToolTip                                                          *m_toolTip;

    ApplController                                                  *m_applController;

    DEVICE_STATE_TYPE_e                                              m_deviceConnectionState[MAX_DEVICES];
    STYLE_TYPE_e                                                     m_currentStyle;
    DISPLAY_CONFIG_t                                                 m_currentDisplayConfig;
    DISPLAY_TYPE_e                                                   m_currentDisplayType;

    quint16                                                          m_startx, m_starty;
    quint8                                                           m_deviceCount;
    quint8                                                           m_cameraCount[MAX_DEVICES];
    quint8                                                           m_camAssignedCount;
    qint16                                                           m_firstIndex, m_lastIndex;
    QString                                                          m_dispDeviceName;
    QStringList                                                      m_deviceNameList, m_cameraNameList;
    quint8                                                           m_cameraIndex[MAX_DEVICES][MAX_CAMERAS];
    quint16                                                          m_deviceStartIndex[MAX_DEVICES];
    CAMERALIST_CALLED_BY_e                                           m_parentClass;
    quint16                                                          m_windowIndex;
    quint16                                                          m_totalElement;

public:
    CameraList(quint16 startx,
               quint16 starty,
               QWidget *parent = 0,
               int indexInPage = 0,
               CAMERALIST_CALLED_BY_e parentClass = MAX_CALLED_BY_CLASS,
               quint16 windowIndex = MAX_CHANNEL_FOR_SEQ);     //1 for display  2 for windowseq
    ~CameraList();

    void getEnabledDevices();
    void updateDeviceList (QString devName,DEVICE_STATE_TYPE_e state);
    void updateCameraList(quint8 deviceIndex, bool isCreationNeeded = true);
    void updateCameraListConnectionState();
    void updateCurrDeviceCamStatus();
    void getAllCameraNamesOfDevice(int deviceIndex);
    void setCameraListGeometry();
    void updateCameraCurrentState(quint8 deviceIndex,
                                  quint8 cameraId,
                                  CAMERA_STATE_TYPE_e currentState);
    void updateDeviceCurrentState(QString devName,
                                  DEVICE_STATE_TYPE_e currentState);
    bool findWindowIndexOfDisplayInfo(quint8 deviceindex,
                                      quint8 cameraIndex,
                                      quint16 &windowIndex,
                                      quint8 &channelIndex);
    bool getCurrSelectedDeviceName(QString deviceName);
    void resetGeometryForScroll();

    void setCurrentStyle(STYLE_TYPE_e styleType);
    void setCurrentDisplayType(DISPLAY_TYPE_e displayType);
    void setCurrentDisplayConfig(DISPLAY_CONFIG_t displayConfig);

    void takeUpKeyAction();
    void takeDownKeyAction();
    void forceFocusToPage(bool isFirstElement);
    virtual void navigationKeyPressed(QKeyEvent *event);
    void wheelEvent(QWheelEvent *event);
    void deleteObject(int deviceIndex);
    void updateIndex();
    quint8 getIndexofdeivce(QString deviceName);

signals:
    void sigUpdateCurrentElement(int index);
    void sigCameraButtonClicked(quint8 cameraIndex,
                                QString deviceName,
                                CAMERA_STATE_TYPE_e connectionState,
                                bool pageSwitchFlag,
                                bool isChangeSelection = false);
    void sigCameraButtonClickedWinSeq(quint8 cameraIndex,
                                      QString deviceName,
                                      CAMERA_STATE_TYPE_e connectionState,
                                      CAMERA_STATE_TYPE_e clearConnectionState = MAX_CAMERA_STATE);
    void slotCameraButtonClickedAddCam(int index,
                                 CAMERA_STATE_TYPE_e connectionState = MAX_CAMERA_STATE);

 void sigClosePage(TOOLBAR_BUTTON_TYPE_e);
 void sigCameraConfigListUpdate();
 void sigSwapWindows(quint16, quint16);
 void sigStartStreamInWindow(DISPLAY_TYPE_e displayType,
                             QString deviceName,
                             quint8 channelId,
                             quint16 windowId);



public slots:
    void slotCameraButtonClicked(int index,
                                 CAMERA_STATE_TYPE_e connectionState = MAX_CAMERA_STATE, int deviceIndex=-1);
    void slotDeviceButtonCliked(int index);
    void slotDevStateChangeBtnClick(int index,
                                    DEVICE_STATE_TYPE_e connectionState);
    void slotScroll(int numberOfSteps);
    void slotShowHideTooltip(quint16 startX,
                             quint16 startY, int deviceIndex,
                             int index,
                             bool toShowTooltip);
};

#endif // CAMERALIST_H
