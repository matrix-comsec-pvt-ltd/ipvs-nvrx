#ifndef PTZCONTROL_H
#define PTZCONTROL_H

#include <QTime>
#include <QWidget>

#include "DataStructure.h"
#include "ApplController.h"
#include "PayloadLib.h"

#include "Controls/Rectangle.h"
#include "Controls/Closebuttton.h"
#include "Controls/Heading.h"
#include "Controls/Bgtile.h"
#include "Controls/ImageControls/Image.h"
#include "Controls/DropDown.h"
#include "Controls/CnfgButton.h"
#include "Controls/ElementHeading.h"
#include "Controls/TextBox.h"
#include "Controls/ToolTip.h"
#include "Controls/ProcessBar.h"
#include "Controls/InfoPage.h"
#include "Controls/TextLabel.h"

typedef enum
{
    PTZ_CTRL_CLOSE_BUTTON,
    PTZ_CTRL_RESUME_BUTTON,
    PTZ_CTRL_MANUAL_START,

    PTZ_CTRL_TOP_KEY,
    PTZ_CTRL_LEFT_KEY,
    PTZ_CTRL_RIGHT_KEY,
    PTZ_CTRL_BOTTOM_KEY,

    PTZ_CTRL_TOP_LEFT_KEY,
    PTZ_CTRL_TOP_RIGHT_KEY,
    PTZ_CTRL_BOTTOM_LEFT_KEY,
    PTZ_CTRL_BOTTOM_RIGHT_KEY,

    PTZ_CTRL_SPEED_DROPDOWNBOX,

    PTZ_CTRL_ZOOM_OUT_KEY,
    PTZ_CTRL_ZOOM_KEY,
    PTZ_CTRL_ZOOM_IN_KEY,

    PTZ_CTRL_FOCUS_OUT_KEY,
    PTZ_CTRL_FOCUS_KEY,
    PTZ_CTRL_FOCUS_IN_KEY,

    PTZ_CTRL_IRIS_OUT_KEY,
    PTZ_CTRL_IRIS_KEY,
    PTZ_CTRL_IRIS_IN_KEY,

    PTZ_CTRL_SET_POS_DROPDOWNBOX,
    PTZ_CTRL_GO_BUTTON,
    PTZ_CTRL_SET_POSNAME_TEXTBOX,

    PTZ_CTRL_SAVE_BUTTON,
    PTZ_CTRL_DELETE_BUTTON,

    MAX_PTZ_CTRLS
}PTZ_CTRL_e;

typedef enum
{
    PTZ_CTRL_LABLE_RESUME_PAUSED_TOUR = 0,
    PTZ_CTRL_LABLE_RESUME,
    PTZ_CTRL_LABLE_MANUAL_TOUR,
    PTZ_CTRL_LABLE_MANUAL_TOUR_STATUS_START,
    PTZ_CTRL_LABLE_MANUAL_TOUR_STATUS_STOP,
    PTZ_CTRL_LABLE_SPEED,
    PTZ_CTRL_LABLE_PRESET_POSITION_SETTINGS,
    PTZ_CTRL_LABLE_POSITION_NUMBR,
    PTZ_CTRL_LABLE_GO,
    PTZ_CTRL_LABLE_POSITION_NAME,
    PTZ_CTRL_LABLE_SAVE,
    PTZ_CTRL_LABLE_DELETE,

    PTZ_CTRL_LABLE_MAX
}PTZ_CTRL_LABLE_e;

class PTZControl : public Rectangle
{
    Q_OBJECT
public:
    explicit PTZControl(qint32 startx, qint32 starty, QString deviceName, quint8 cameraNum, QWidget *parent = 0);
    ~PTZControl();

    void createDefaultComponent();
    void createPTZControls();
    void hideAllTooltips();

    void getManualTourStatus();

    void createPayload(REQ_MSG_ID_e msgType);
    void createCmdPayload(SET_COMMAND_e cmdType, quint8 totalFields = 0);

    void getPositionConfig();
    bool savePositiontoConfig();
    void deletePositionFromConfig();

    void getTourStatus();
    void sendStartManualTourCmd();
    void sendStopManualTourCmd();
    void sendResumeTourCmd();
    void updateManualTourStatus(QString deviceName,quint8 camNum, LOG_EVENT_STATE_e evtState);

    void sendPresetPosGoCmd();
    void sendPanTiltCmd(quint8 panVal, quint8 tiltVal);

    void sendZoomCmd(quint8 zoomVal);
    void sendFocusCmd(quint8 focusVal);
    void sendIrisCmd(quint8 irisVal);

    void giveFocusToControl(quint8 index);
    void showToolTipOfControl(quint8 index, bool isMouseHoverOnImage = true);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);

    void takeLeftKeyAction();
    void takeRightKeyAction();
    void takeEnterKeyAction();

    void notifyError();

    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);
    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void functionKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);

    void processDeviceResponse(DevCommParam *param, QString deviceName);

    QString currentDevName();

signals:
    void sigObjectDelete();

public slots:
    void slotButtonClicked(int);
    void slotInfoPageBtnclick(int);
    void slotUpdateCurrentElement(int);
    void slotImageMouseHover(int,bool);
    void slotSpinBoxValueChange(QString,quint32);

private:

    ApplController* applController;
    PayloadLib*     payloadLib;
    QString         currentDeviceName;
    InfoPage*       infoPage;

    bool            isMousePressed;
    bool            isAdvanceMode;
    bool            isDeleteCmd;

    QPoint          pressedPoint;
    qint32          startX;
    qint32          startY;

    CloseButtton*   closeButton;
    Heading*        ptzControlHeading;

    BgTile*         topBgTile;

    Image*          topKeyImageControl;
    Image*          topLeftKeyImageControl;
    Image*          topRightKeyImageControl;
    Image*          bottomKeyImageControl;
    Image*          bottomLeftKeyImageControl;
    Image*          bottomRightKeyImageControl;
    Image*          leftKeyImageControl;
    Image*          rightKeyImageControl;

    BgTile*         imageTopBgTile;
    DropDown*       speedDropDownBox;

    Image*          zoomImageControl;
    Image*          zoomInImageControl;
    Image*          zoomOutImageControl;
    ToolTip*        zoomInToolTip;
    ToolTip*        zoomOutToolTip;

    Image*          focusImageControl;
    Image*          focusInImageControl;
    Image*          focusOutImageControl;
    ToolTip*        focusToolTip;
    ToolTip*        focusInToolTip;
    ToolTip*        focusOutToolTip;

    Image*          irisImageControl;
    Image*          irisInImageControl;
    Image*          irisOutImageControl;
    ToolTip*        irisToolTip;
    ToolTip*        irisInToolTip;
    ToolTip*        irisOutToolTip;

    CnfgButton*     goButton;

    BgTile*         manualButtonTile;
    CnfgButton*     manualButton;
    TextLabel*      manualButtonLabel;

    ElementHeading* presetSettingHeading;
    DropDown*       setPositionDropDownBox;
    TextboxParam*   setPositionNameParam;
    TextBox*        setPositionNameTextBox;

    BgTile*         cnfgButtonTile;
    CnfgButton*     saveButton;
    CnfgButton*     deleteButton;

    quint8          manualTourHealthState[MAX_CAMERAS];
    quint8          m_cameraIndex;

    NavigationControl* m_elementList[MAX_PTZ_CTRLS];
    quint8             m_currentElement;
    quint8             currentButtonClick;

    BgTile*         resumeButtonTile;
    CnfgButton*     resumeTourButton;
    TextLabel*      resumeTourLabel;
    QStringList     m_positionNameList;
    quint8          m_currentPositionIndex;
};

#endif // PTZCONTROL_H
