#ifndef TOOLBAR_H
#define TOOLBAR_H

#include "ApplController.h"
#include "Controls/ToolbarButton.h"
#include "Controls/ControlButton.h"
#include "Controls/ReadOnlyElement.h"
#include "Controls/TextLabel.h"
#include "EnumFile.h"
#include "ApplController.h"
#include "PayloadLib.h"
#include "Controls/ToolTip.h"
#include "Controls/PickList.h"
#include "Controls/CnfgButton.h"
#include "KeyBoard.h"
#include "Wizard/SetupWizard.h"

#define CONTROL_ICON_BUTTONS    (STYLE_SELECT_BUTTON)
#define STATUS_ICON_BUTTONS     (MAX_TOOLBAR_BUTTON - CONTROL_ICON_BUTTONS)

typedef enum
{
    MENU_BUTTON = 0,
    STYLE_SELECT_PICKLIST = CONTROL_ICON_BUTTONS,
    PAGE_PREVIOUS,
    PAGE_NEXT,
    STATUS_BUTTON,
    MAX_TOOLBAR_ELEMENT = 24
}TOOLBAR_ELEMENT_e;

typedef enum
{
    CPU_LOAD_0 = 0,
    CPU_LOAD_20,
    CPU_LOAD_40,
    CPU_LOAD_60,
    CPU_LOAD_80,
    CPU_LOAD_100,
    MAX_CPU_LOAD

}CPU_LOAD_STATE_e;

class Toolbar : public KeyBoard
{
    Q_OBJECT
private:
    ToolbarButton* m_toolbarButtons[CONTROL_ICON_BUTTONS];
    ToolbarButton* m_statusIconButtons[STATUS_ICON_BUTTONS];
    ControlButton* m_previousPageButton;
    ControlButton* m_nextPageButton;
    ReadOnlyElement* m_pageNumber;
    PickList* m_styleSelectionPickList;
    TextLabel* m_dateText;
    QTimer* m_updateTimer;
    ToolTip* m_toolTip;
    quint8  m_camCount;
    TextLabel*  m_camCountLabel;
    CnfgButton* m_exitButton;

    ApplController* m_applController;
    PayloadLib* m_payloadLib;

    TOOLBAR_BUTTON_TYPE_e m_currentClickedButton;
    int m_currentElement;
    int m_currentDate, m_currentMonth, m_currentYear, m_currentHour, m_currentMinute, m_currentSecond;

    NavigationControl* m_elementList[MAX_TOOLBAR_ELEMENT];
    bool  m_isCamNotify;
    bool  m_isExpandMode;

    TOOLBAR_BUTTON_TYPE_e m_nextToolbarButtonLoad;
    CPU_LOAD_STATE_e      m_cpuLoadState;
    QTimer*               m_cpuLoadupdateTimer;
    quint8                m_cpuLoad;

public:
    explicit Toolbar(QWidget *parent = 0);
    ~Toolbar();

    void openToolbarPage(TOOLBAR_BUTTON_TYPE_e index);
    void closeToolbarPage(TOOLBAR_BUTTON_TYPE_e index);
    void openNextToolbarPage(void);
    void getDateAndTime(QString deviceName);
    void processDeviceResponse(DevCommParam* param, QString deviceName);
    QString changeToStandardDateTime();
    void updateDateTime(QString dateTimeString);
    void updateDateTime(QDateTime dateTime);
    STATE_TYPE_e getCurrentToolBarButtonState(TOOLBAR_BUTTON_TYPE_e);
    void changePageOnLayoutChange();
    TOOLBAR_BUTTON_TYPE_e getCurrentToolBarButton();
    TOOLBAR_BUTTON_TYPE_e getNextToolBarButton();
    void loadToolbarPage(TOOLBAR_BUTTON_TYPE_e index);
    void unloadToolbarPage();
    void deletePicklist();

    void getCpuLoadUpdate();

    void takeLeftKeyAction();
    void takeRightKeyAction();

    bool eventFilter(QObject *, QEvent *);
    void paintEvent(QPaintEvent *);
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
    void changeNextToolBarBtnLoad(TOOLBAR_BUTTON_TYPE_e index);
    void changeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e buttonIndex, STATE_TYPE_e state);

    void updateGeometry();

    //ketboard methods
    virtual void navigationKeyPressed(QKeyEvent *event);

signals:
    void sigOpenToolbarPage(TOOLBAR_BUTTON_TYPE_e index);
    void sigCloseToolbarPage(TOOLBAR_BUTTON_TYPE_e index);
    void sigChangePage(NAVIGATION_TYPE_e navigationType);

    void sigToolbarApplyStyle(DISPLAY_TYPE_e displayType,
                              DISPLAY_CONFIG_t displayConfig,
                              STYLE_TYPE_e styleNo);
    void sigExitVideoPopupState();
    void sigOpenWizard();

public slots:
    void slotChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e buttonIndex, STATE_TYPE_e state);
    void slotButtonClicked(TOOLBAR_BUTTON_TYPE_e index);
    void slotUpdateDateTime();
    void slotNotifyStatusIcon(TOOLBAR_BUTTON_TYPE_e index, bool state);
    void slotClosePage(TOOLBAR_BUTTON_TYPE_e index);
    void slotUpdateCurrentElement(int index);
    void slotChangePageFromLayout();
    void slotChangePage(int index);
    void slotShowHideTooltip(int index, bool toShowTooltip);
    void slotStyleChanged(quint8 index,QString value, int indexInPage);
    void slotStyleChnagedNotify(STYLE_TYPE_e styleNo);
    void slotPicklistLoad(quint8);
    void slotShowHideStyleAndPageControl(bool toShow, quint8 type);
    void slotUpdateCpuLoad();
    void slotCfgButtonClicked(int index);
    void slotChangeToolbarBtnState(TOOLBAR_BUTTON_TYPE_e index,bool backupState);
    void slotChangeMuteUnmute();
};

#endif // TOOLBAR_H
