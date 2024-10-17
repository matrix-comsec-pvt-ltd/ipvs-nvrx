#ifndef DISPLAYSETTING_H
#define DISPLAYSETTING_H

#include "ApplController.h"
#include "Controls/BackGround.h"
#include "Controls/PickList.h"
#include "Controls/PageOpenButton.h"
#include "Controls/Bgtile.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/LayoutList.h"
#include "Controls/LayoutCreator.h"
#include "Controls/SpinBox.h"
#include "Controls/CnfgButton.h"
#include "Controls/CameraList.h"
#include "Controls/ReadOnlyElement.h"
#include "Controls/ControlButton.h"
#include "Controls/StyleSelectionOpt.h"
#include "Controls/ProcessBar.h"
#include "Controls/InfoPage.h"
#include "Controls/AppearanceSetting.h"
#include "Controls/WindowSequenceSettings.h"
#include "Controls/TextWithBackground.h"
#include "Controls/DropDown.h"
#include "ManagePages/UserValidation.h"
#include "PayloadLib.h"

#define MAX_PAGE_NUMBER 4

typedef enum
{
    DISPLAY_STG_CLOSE_BTN,
    DISPLAY_STG_LIVE_VIEW_DROPDOWN,
    DISPLAY_STG_RESOLUTION_PICKLIST,
    DISPLAY_STG_TV_ADJUST,
    DISPLAY_STG_BANDWITH_OPT,
    DISPLAY_STG_APPEARANCE_BTN,

    DISPLAY_STG_LAYOUT_LIST,
    DISPLAY_STG_LAYOUT_CREATOR,

    DISPLAY_STG_FIRST_PAGE,
    DISPLAY_STG_PREV_PAGE,
    DISPLAY_STG_PAGE_NUM_BTN,
    DISPLAY_STG_NEXT_PAGE = (DISPLAY_STG_PAGE_NUM_BTN + MAX_PAGE_NUMBER),
    DISPLAY_STG_LAST_PAGE,

    DISPLAY_STG_CAMERA_LIST,

    DISPLAY_STG_LAYOUT_STYLE_PICKLIST,
    DISPLAY_STG_DEFAUILT_SEL,
    DISPLAY_STG_SEQ_ENABLE,
    DISPLAY_STG_SEQ_INTERVAL_SPINBOX,
    DISPLAY_STG_APPLY_BTN,
    DISPLAY_STG_SAVE_BTN,
    DISPLAY_STG_CANCEL_BTN,

    MAX_DISPLAY_STG_ELEMENTS
}DISPLAY_STG_ELEMENTS_e;


class DisplaySetting : public BackGround
{
    Q_OBJECT
public:

    explicit DisplaySetting(QWidget * parent);
    ~DisplaySetting();

    void updateDeviceState(QString devName, DEVICE_STATE_TYPE_e devState);
    void showEvent(QShowEvent * event);

    //keyboard support added
    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);
    virtual void tabKeyPressed(QKeyEvent *event);
    virtual void backTab_KeyPressed(QKeyEvent *event);
    virtual void ctrl_S_KeyPressed(QKeyEvent *event);

signals:
    void sigChangeLayout(LAYOUT_TYPE_e index, DISPLAY_TYPE_e disId, quint16 windowIndex, bool ifAtcualWindow, bool ifUpdatePage);
    void sigProcessApplyStyleToLayout(DISPLAY_TYPE_e displayType, DISPLAY_CONFIG_t m_displayConfig, STYLE_TYPE_e styleNo);
    void sigToolbarStyleChnageNotify(STYLE_TYPE_e styleNo);
    void sigliveViewAudiostop();

public slots:
    void slotUpadateCurrentElement(int index);
    void slotPickListButtonClick(int index);
    void slotSettingOptSelButtonClicked(OPTION_STATE_TYPE_e state,int index);
    void slotPicklistValueChanged(quint8 index, QString value, int indexInPage);
    void slotChangeLayout(LAYOUT_TYPE_e layoutId);
    void slotCnfgButtonClick(int index);
    void slotInfoPageCnfgBtnClick(int index);
    void slotNextPrevPageClick(int index);
    void slotStyleSelCnfgBtnClick(int index,quint8 styleNo);

    void slotWindowSelected(quint16 index);
    void slotWindowImageClicked(WINDOW_IMAGE_TYPE_e imageType, quint16 windowIndex);
    void slotWindowImageHover(WINDOW_IMAGE_TYPE_e imageType, quint16 windowIndex, bool isHover);
    void slotSwapWindows(quint16 firstWindow, quint16 secondWindow);
    void slotDragStartStopEvent(bool isStart);

    void slotWindowResponseToDisplaySettingsPage(DISPLAY_TYPE_e displayId, QString deviceName, quint8 cameraId, quint16 windowId);
    void slotAppearanceButtonClick(int);
    void slotObjectDelete();
    void slotLayoutResponseOnApply(DISPLAY_TYPE_e displayType, bool isCurrentStyle);
    void slotCameraButtonClicked(quint8 cameraIndex, QString deviceName, CAMERA_STATE_TYPE_e connectionState,
                                 bool pageSwitchFlag, bool isChangeSelection);
    void slotPageNumberButtonClick(QString);
    void slotOkButtonClicked (QString username, QString password);
    void slotCameraConfigListUpdate();
    void slotSpinBoxValueChanged(QString,quint32 index);

private:
    ProcessBar*             m_processBar;
    InfoPage*               m_infoPage;

    QString                 m_imageSource;
    Image*                  m_backGroundImage;

    StyleSelectionOpt*      m_styleSelectionOpt;
    PickList*               m_resolutionPicklist;
    PageOpenButton*         m_appearancePageopenBtn;

    BgTile*                 m_layoutListBackground;
    DropDown*               m_LiveViewDropdown;

    LayoutList*             m_layoutList;
    LayoutCreator*          m_layoutCreator;

    UsersValidation*        m_userValidation;

    BgTile*                 m_camListBackground;
    CameraList*             m_cameraList;

    PayloadLib*             m_payloadLib;
    OptionSelectButton*     m_bandwidthOpt;

    PickList*               m_stylePicklist;
    OptionSelectButton*     m_dfltOptSelBtn;
    OptionSelectButton*     m_seqOptSelBtn;
    SpinBox*                m_seqIntervalSpinbox;

    CnfgButton*             m_applyBtn;
    CnfgButton*             m_saveBtn;
    CnfgButton*             m_cancelBtn;

    ReadOnlyElement*        m_pageNoReadOnly;
    ControlButton*          m_firstPageCntrlBtn;
    ControlButton*          m_lastPageCntrlBtn;
    ControlButton*          m_prevPageCntrlBtn;
    ControlButton*          m_nextPageCntrlBtn;

    SpinBox*                m_tvAdjustSpinBox;

    AppearanceSetting*      m_appearanceSetting;
    WindowSequenceSettings* m_windowSequenceSetting;
    TextWithBackground*     m_PageNumberLabel[MAX_PAGE_NUMBER];

    ApplController*         m_applController;
    NavigationControl*      m_elementList[MAX_DISPLAY_STG_ELEMENTS];

    DEV_TABLE_INFO_t        m_devTable;
    STYLE_TYPE_e            m_currDfltStyle;
    LAYOUT_TYPE_e           m_currLayout;
    DISPLAY_TYPE_e          m_currentDisplayId;
    STYLE_TYPE_e            m_currentStyle;
    DISPLAY_CONFIG_t        m_displayConfig[MAX_DISPLAY_TYPE];
    DISPLAY_RESOLUTION_e    prevResolution;
    DISPLAY_RESOLUTION_e    currResolution;

    bool                    m_isChangeDone;
    bool                    nextPageSelected;
    quint16                 m_currPage;
    quint16                 m_currentWindow;
    quint16                 maxWindows;
    int                     m_currElement;
    MAIN_DISPLAY_TYPE_e     m_currentdisplayIndex;

    quint16                 m_tempStartAudioInWindow;
    QString                 m_username, m_password;

    void createDefaultComponent();
    bool setTVApperanceParameter();
    void getConfig();
    void saveConfig(quint8 styleToSave);
    void getDefaultStyle();
    void displayLayoutcreatorForCurrentPage();
    void applyChangesInStyle();
    bool saveBandwidthOptimizeFlag();
    void saveLiveViewType(void);
    bool isWindowWiseSequeningConfigure ();

    void doChangeDisplayTypeProcess();
    bool findWindowIndexOfDisplayInfo(quint8 cameraIndex, QString deviceName, quint16 &windowIndex,quint8 &channelIndex);
    bool findFreeWindowOfDisplayInfo(quint16 &windowIndex);
    void windowCloseButtonClicked(quint16 windowIndex);
    void sequenceConfigButtonClicked(quint16 windowIndex);
    bool isMultipleChannelAssigned(quint16 windowIndex);
    void takeLeftKeyAction();
    void takeRightKeyAction();
    void updatePageNumbers();
    void audioStopOnApplyCondition();
    void restartLocalClient();

public:
    static bool pageSequenceStatus;
    void processDeviceResponse(DevCommParam *param, QString deviceName);
};

#endif // DISPLAYSETTING_H
