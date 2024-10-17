#ifndef MXQUICKBACKUP_H
#define MXQUICKBACKUP_H


#include "ApplController.h"
#include "NavigationControl.h"
#include "ApplicationMode.h"

#include "Controls/BackGround.h"
#include "Controls/DropDown.h"
#include "Controls/ControlButton.h"
#include "Controls/TableCell.h"
#include "Controls/Heading.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/ReadOnlyElement.h"
#include "Controls/CnfgButton.h"
#include "Controls/PlaybackRecordData.h"
#include "PayloadLib.h"

#include "Controls/TextLabel.h"
#include "Controls/InfoPage.h"
#include "Controls/ProcessBar.h"
#include "Controls/TileWithText.h"
#include "UsbControl.h"
#include "Controls/MxTimelineSlider.h"
#include "Controls/MxQuickBckupReportTable.h"
#include "Controls/ElementHeading.h"
#include "Controls/PageOpenButton.h"
#include "Controls/TextBox.h"

#define MAXIMUM_REC_ALLOWED                 100

typedef enum
{
    QUICK_BACKUP_START_BTN,
    QUICK_BACKUP_VIEW_STATUS_BTN,
    QUICK_BACKUP_SHOW_CLIP_SETTNGS_TILE,
    QUICK_BACKUP_HIDE_CLIP_SETTNGS_TILE,
    QUICK_BACKUP_DEVICE_SPINBOX,
    QUICK_BACKUP_CAM_LIST,
    QUICK_BACKUP_CAMERASELECT_CHECKBOX,
    QUICK_BACKUP_SCROLLBAR =(QUICK_BACKUP_CAMERASELECT_CHECKBOX + MAX_CAMERAS + 1),
    QUICK_BACKUP_BCKP_LOCATION,
    QUICK_BACKUP_FILE_FORMAT,    
    QUICK_BACKUP_MINIMIZE_BTN,
    QUICK_BACKUP_CLOSE_BTN,

    QUICK_BACKUP_HOURS_DROPDOWN,
    QUICK_BACKUP_ADVANCE_BUTTON,
    QUICK_BACKUP_TIMELINE_SLIDER,
    QUICK_BACKUP_TIMELINE_PREV,
    QUICK_BACKUP_TIMELINE_NEXT,

    MAX_QUICK_BACKUP_ELEMENTS

}QUICK_BACKUP_ELEMENTS_e;

typedef enum
{
    FIELD_INDEX_NO,
    FIELD_STRAT_TIME,
    FIELD_END_TIME,
    FIELD_CAM_NO,
    FIELD_EVT_TYPE,
    FIELD_OVERLAP,
    FIELD_HDD,
    FIELD_PARTION,
    FIELD_STORAGE,

    MAXIMUM_FIELD_NO
}CNFG_FIELDS_e;

typedef enum
{
    QB_IN_PROGRESS,
    QB_SUCCESS,
    QB_FAILED,

    MAX_STATUS
}QUICK_BACKUP_STATUS_e;

typedef struct
{
    quint8      camNo;
    quint8      noofRecords;
    QString     cameraName;
    QString     status;
    QString     reason;

}QUICK_BACKUP_REPORT_DATA_t;


class MxQuickBackup : public BackGround
{
    Q_OBJECT
private:
    bool                isShowClipSettings, m_isDeviceConnected;
    ApplController      *applcontroller;
    DropDown            *devNameDropDownBox, *backupLocationDropDownBox;
    OptionSelectButton  *m_cameraCheckbox[MAX_CAMERAS + 1];
    CnfgButton          *startButton,*viewStatusButton;

    BgTile              *complRect;
    TileWithText        *clipSettingsTile, *cameraSelBgTile;
    TextLabel           *clipSettingsBgTileLabel, *deviceDropDownBoxLabel, *cameraListLabel,
                        *fileFormatLabel, *backupLocationLabel, *cameraSelBgTitle;
    TextLabel           *m_advanceOptionLable;

    int                 m_currElement;
    quint8              m_firstCameraIndex, m_lastCameraIndex;
    quint8              totalRec, searchRecIndex, currBckpLocationIndex;
    quint8              tempCamNumber, tempRecFound;
    quint8              totalCamCount;
    quint8              success;
    quint8              fail;
    quint8              currCamNo;
    QString             m_currDevName, searchDevName;
    Image               *clipSettingsImage,*minimizeButtonImage, *disabledImage;
    NavigationControl   *m_elementList[MAX_QUICK_BACKUP_ELEMENTS];
    ScrollBar           *m_Scrollbar;
    quint8              m_cameraIndex[MAX_CAMERAS];
    QStringList         cameraList;
    MxTimelineSlider*   m_timeLineSlider;
    bool                m_isminimize, m_isBackupOn;
    MxQuickBckupReportTable*       m_quickBcpTable;
    InvisibleWidgetCntrl* m_inVisibleWidget;
    TextLabel           *backupStatusLabel;
    ToolTip*            m_sliderTileToolTip;
    ToolTip*            m_sliderToolTip;

    PlaybackRecordData  playbackRecData[MAXIMUM_REC_ALLOWED];
    PayloadLib          *payloadLib;
    bool                recSelect[MAX_CAMERAS+1], isBackupReqSent, isbackupcompleted;
    bool                forQuickbackup;
    quint8              arrIndexofrecData[MAXIMUM_REC_ALLOWED];
    QString             m_startDtTmStr;
    QString             m_endDtTmStr;
    QDateTime           m_startDtTm;
    QDateTime           m_endDtTm;

    DropDown*           m_HourDropDownBox;
    QMap<quint8, QString>   modeList;

    QUICK_BACKUP_REPORT_DATA_t  statusReport[MAX_CAMERAS + 1];
    PageOpenButton      *m_advanceOptionButton;
    TextboxParam        *fileFormatTextboxParam;
    TextBox             *fileFormatTextBox;

public:
    explicit MxQuickBackup(QWidget* parent = 0);
    ~MxQuickBackup();

    void createDefaultComponent ();
    void showHideElements(bool flag);
    void changeCameraElement();
    void selectDeselectAllRecords(OPTION_STATE_TYPE_e state);
    bool isAllButtonChecked();
    void resetGeometryForCameraOption();
    bool getMinimizeFlag();
    void setMinimizeFlag(bool state);
    bool getBackupFlag();
    void setBackupFlag(bool state);
    void processDeviceResponse(DevCommParam *param, QString deviceName);
    void processDeviceResponseforSliderTime(DevCommParam *param, QString deviceName);
    void searchRecData();
    void sendMultipleRecBackupCmd();
    void fillMultipleRecBackupCmd ();
    void BkpSysEvtAction(QString devName, LOG_EVENT_STATE_e evtState);
    void sendStopBackupCmd();
    void initRecData();
    bool getQuickBackupFlag();
    void fillReportTable();
    void updateChangeCameraElement();
    void updateDeviceList(void);

    void wheelEvent(QWheelEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);
    virtual void tabKeyPressed(QKeyEvent *event);
    virtual void backTab_KeyPressed(QKeyEvent *event);
    virtual void navigationKeyPressed(QKeyEvent *event);
    void takeLeftKeyAction();
    void takeRightKeyAction();
    void takeUpKeyAction();
    void takeDownKeyAction();


signals:
    void sigChangeToolbarBtnState(TOOLBAR_BUTTON_TYPE_e index,bool backupState);
    void sigUpdateQuickbackupFlag(bool flag);
    void sigAdavnceOptions (QUICK_BACKUP_ELEMENTS_e index);


public slots:
    void slotImageClicked(int indexInPage);
    void slotUpdateCurrentElement (int index);
    void slotSpinboxValueChanged(QString str,quint32 index);
    void slotCameraCheckboxClicked(OPTION_STATE_TYPE_e currentState, int indexInPage);    
    void slotScrollbarClick(int numberOfSteps);
    void slotButtonClick(int index);
    void slotQuickBkupRptClose(TOOLBAR_BUTTON_TYPE_e);
    void slotTileToolTipUpdate(QString m_timeString, int curX, int curY);
    void slotTileToolTipHide(bool toolTip);
    void slotSliderToolTipUpdate(QString m_SliderDateTime,int curX,int curY);
    void slotSliderToolTipHide(bool sliderToolTip);
    void slotCloseBtnClick(int indexInPage);
    void slotTileWithTextClick(int indexInPage);
    void slotAdvanceButtonClicked(int index);
};

#endif      //MXQUICKBACKUP_H
