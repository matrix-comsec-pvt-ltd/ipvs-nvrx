#ifndef AUTOADDCAMERALIAT_H
#define AUTOADDCAMERALIST_H

#include <QWidget>
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

#define MAX_RECORD_DATA_AUTO_ADD         8
#define MAX_RECORD_FEILDS_AUTO_ADD       5

typedef enum
{
    FILTER_NONE,
    FILTER_ADDED,
    FILTER_NOT_ADDED

}SEARCH_FILTER_TYPE_e;

class AutoAddCameraList : public KeyBoard, public NavigationControl
{
    Q_OBJECT
public:
    explicit AutoAddCameraList(QString currDevName,QWidget* parent = 0, PayloadLib *payloadLib=NULL);
    ~AutoAddCameraList();

    void processDeviceResponse (DevCommParam *param, QString deviceName);
    void handleInfoPageMessage (int index);
    void paintEvent (QPaintEvent *);

    void navigationKeyPressed(QKeyEvent *event);
    void backTab_KeyPressed(QKeyEvent *event);
    void tabKeyPressed(QKeyEvent *event);
    void escKeyPressed(QKeyEvent *event);
    void insertKeyPressed(QKeyEvent *event);
    void takeLeftKeyAction();
    void takeRightKeyAction();

signals:
    void sigObjectDelete(bool);

public slots:
    void slotButtonClick(int);
    void slotOptionSelectionButton(OPTION_STATE_TYPE_e,int);
    void slotInfoPageBtnclick(int index);
    void slotUpdateCurrentElement(int index);

private:
    // private variables
    bool isCancelSend;

    ApplController*     applController;
    CloseButtton*       m_closeButton;
    DEV_TABLE_INFO_t    devTableInfo;

    NavigationControl*  m_elementList[100];
    quint8              m_currentElement;
    quint8              currentPageNo;
    quint8              maximumPages;
    quint8              maxSearchListCount;

    QStringList         camStatusList;
    QStringList         macAddressList;
    QStringList         camIpAddrList;
    QStringList         modelList;

    QStringList         AddCamList;
    QStringList         AddCamIpAddrList;
    QStringList         AddCamModelList;

    // Controls
    BgTile*             topBgtile;
    BgTile*             bottomBgtile;
    ProcessBar*         processBar;

    OptionSelectButton* selectAllCam;
    OptionSelectButton* selectCam[MAX_RECORD_DATA_AUTO_ADD];
    TableCell*          fieldsHeading[MAX_RECORD_FEILDS_AUTO_ADD];
    TableCell*          srNumber[MAX_RECORD_DATA_AUTO_ADD];
    TableCell*          macAddress[MAX_RECORD_DATA_AUTO_ADD];

    TableCell*          ipAddress[MAX_RECORD_DATA_AUTO_ADD];
    TableCell*          model[MAX_RECORD_DATA_AUTO_ADD];
    TableCell*          status[MAX_RECORD_DATA_AUTO_ADD];    

    TextLabel*          fieldsHeadingStr[MAX_RECORD_DATA_AUTO_ADD];
    TextLabel*          macAddressStr[MAX_RECORD_DATA_AUTO_ADD];

    TextLabel*          ipAddrStr[MAX_RECORD_DATA_AUTO_ADD];
    TextLabel*          modelStr[MAX_RECORD_DATA_AUTO_ADD];
    TextLabel*          statusStr[MAX_RECORD_DATA_AUTO_ADD];

    ControlButton*      previousButton;
    ControlButton*      nextButton;

    CnfgButton*         addButton;
    CnfgButton*         rejectButton;
    PayloadLib*         m_payloadLib;
    Rectangle*          m_backGround;
    QString             currDevName;
    Heading*            m_heading;
    InfoPage*           infoPage;

    // private Fuction
    void createDefaultComponent();
    void updateList(DevCommParam *param);
    void sendCommand(SET_COMMAND_e cmdType, quint8 totalfeilds = 0);
    void clearAllList();
    void clearSerachDisplayList (quint8 recordOnPage);
    void showCameraSearchList();
    void updateNavigationControlStatus ();
    void fillAddCamList ();
    void updateSelCamState (bool isStateOn = false);
};

#endif // AUTOADDCAMERALIST_H
