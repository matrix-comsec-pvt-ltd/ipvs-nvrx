#ifndef PBTOOLBAR_H
#define PBTOOLBAR_H

#include <QWidget>
#include "NavigationControl.h"
#include "ApplController.h"

#include "Controls/PlaybackControl/PbToolbarButton.h"
#include "Controls/Closebuttton.h"
#include "Controls/SliderControl.h"
#include "Controls/Rectangle.h"
#include "Controls/Bgtile.h"
#include "Controls/ToolTip.h"
#include "Controls/TextLabel.h"

typedef enum
{
    PB_TOOLBAR_ELE_PLAY_BTN,
    PB_TOOLBAR_ELE_STOP_BTN,
    PB_TOOLBAR_ELE_REV_PLAY_BTN,
    PB_TOOLBAR_ELE_SLOW_BTN,
    PB_TOOLBAR_ELE_FAST_BTN,
    PB_TOOLBAR_ELE_PREVIOUS_BTN,
    PB_TOOLBAR_ELE_NEXT_BTN,
    PB_TOOLBAR_ELE_MUTE_BTN,
    MAX_PB_TOOLBAR_ELE_BTN,

    PB_TOOLBAR_ELE_SLIDER = MAX_PB_TOOLBAR_ELE_BTN,
    PB_TOOLBAR_ELE_CLOSE,

    MAX_PB_TOOLBAR_ELEMENTS
}PB_TOOLBAR_ELEMENTS_e;

class PbToolbar : public KeyBoard
{
    Q_OBJECT

private:
    quint16 m_windowId;
    int m_sliderActiveWidth;

    int toolBarStartX;
    int toolBarStartY;
    int toolBarWidth;

    quint64 m_recStartTime;
    quint64 m_recEndTime;
    quint64 m_recCurrTime;
    quint64 m_totalTimeDiff;

    bool m_buttonState[MAX_PB_TOOLBAR_ELE_BTN];
    PB_TOOLBAR_SIZE_e m_toolBarSize;
    PbToolbarButton *toolBarBtn[MAX_PB_TOOLBAR_ELE_BTN];
    CloseButtton *closeBtn;
    Rectangle *sliderRect;
    SliderControl *slider;

    BgTile *closeBtnBg;
    ToolTip *timeTooltip;
    ApplController *applController;

    NavigationControl* m_elementList[MAX_PB_TOOLBAR_ELEMENTS];
    int m_currElement;

    TextLabel* m_pbSpeed;
    TextLabel* m_pbDateTime;

public:
    PbToolbar(int startX,
              int startY,
              int windowWidth,
              int windowHeight,
              quint16 windowId,
              quint8 toolbarSize,
              quint64 startTime,
              quint64 endTime,
              quint64 currTime,
              QWidget *parent = 0);
    ~PbToolbar();

    void createDefaultComponent();
    void changeStateOfButton(PB_TOOLBAR_ELEMENTS_e index, bool state, bool tooltipUpdation = true);
    bool getStateOfButton(PB_TOOLBAR_ELEMENTS_e index);
    quint16 getPbToolbarWinId();
    void updateCurrTime(quint64 currTime);
    void changeButtonEnableState(PB_TOOLBAR_ELEMENTS_e button,bool isEnable);
    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void asciiCharKeyPressed(QKeyEvent *event);
    virtual void functionKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);    
    virtual void insertKeyPressed(QKeyEvent *event);
    void takeLeftKeyAction();
    void takeRightKeyAction();
    void takeUpKeyAction();
    void takeDownKeyAction();
    void takeNextKeyAction();
    void takePrevKeyAction();

    void keyHandling(PB_TOOLBAR_ELEMENTS_e index);
    void showPbSpeedDateTime(quint16 arrayWindowIndex);

signals:
    void sigSliderValueChanged(quint64 currTime, quint16 windowId);
    void sigPbToolbarBtnClick(int index, quint16 windowId, bool state);

public slots:
    void slotPbBtnButtonClick(int index);
    void slotPbSliderValueChanged(int currVal ,int);
    void slotHoverOnSlider(int currVal,int index);
    void slotHoverInOutOnSlider(bool hoverState, int index);
    void slotUpadateCurrentElement(int index);
    void slotImageMouseHover(quint8,bool);
};

#endif // PBTOOLBAR_H
