#ifndef INSTANTPLAYBACK_H
#define INSTANTPLAYBACK_H

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
    INST_PB_TOOLBAR_ELE_PLAY_BTN,
    INST_PB_TOOLBAR_ELE_REV_PLAY_BTN,
    INST_PB_TOOLBAR_ELE_STOP_BTN,
    INST_PB_TOOLBAR_ELE_MUTE_BTN,
    MAX_INST_PB_TOOLBAR_ELE_BTN,

    INST_PB_TOOLBAR_ELE_SLIDER = MAX_INST_PB_TOOLBAR_ELE_BTN,
    INST_PB_TOOLBAR_ELE_CLOSE,

    MAX_INST_PB_TOOLBAR_ELEMENTS
}INST_PB_TOOLBAR_ELEMENTS_e;

class InstantPlayback : public KeyBoard
{
    Q_OBJECT
public:
    explicit InstantPlayback(quint16 startX,
                             quint32 startY,
                             quint16 windowWidth,
                             quint16 windowHeight,
                             quint16 windowId,
                             quint8 toolbarSize,
                             quint64 startTime,
                             quint64 endTime,
                             quint64 currTime, QWidget *parent = 0);
    ~InstantPlayback();
    
    void showInstantPbDateTime();
    void changeStateOfButton(INST_PB_TOOLBAR_ELEMENTS_e index, bool state,
                             bool tooltipUpdation = true);
    bool getStateOfButton(INST_PB_TOOLBAR_ELEMENTS_e index) const;
    quint16 getPbToolbarWinId() const;

    void updateCurrTime(quint64 currTime);

    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void asciiCharKeyPressed(QKeyEvent *event);
    virtual void functionKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);
    virtual void tabKeyPressed(QKeyEvent *event);
    virtual void insertKeyPressed(QKeyEvent *event);


signals:
    void sigSliderValueChanged(quint64 currTime, quint16 windowId);
    void sigInstantPbToolbarBtnClick(int index, quint16 windowId, bool state);
    
public slots:
    void slotButtonClick(int);
    void slotPbSliderValueChanged(int currVal ,int);
    void slotHoverOnSlider(int currVal,int index);
    void slotHoverInOutOnSlider(bool hoverState, int index);
    void slotUpadateCurrElement(int index);
    void slotImageMouseHover(quint8,bool);

private:

    // private Variables
    quint16 m_windowId;
    qint32 m_sliderActiveWidth;

    quint32 toolBarStartX;
    quint32 toolBarStartY;
    quint32 toolBarWidth;

    quint64 m_recStartTime;
    quint64 m_recEndTime;
    quint64 m_recCurrTime;
    quint64 m_totalTimeDiff;

    bool m_buttonState[MAX_INST_PB_TOOLBAR_ELE_BTN];
    PB_TOOLBAR_SIZE_e m_toolBarSize;
    PbToolbarButton *toolBarBtn[MAX_INST_PB_TOOLBAR_ELE_BTN];
    CloseButtton *closeBtn;
    Rectangle *sliderRect;
    SliderControl *slider;

    BgTile *closeBtnBg;
    ToolTip *timeTooltip;
    ApplController *applController;

    NavigationControl* m_elementList[MAX_INST_PB_TOOLBAR_ELEMENTS];
    quint32 m_currElement;

    TextLabel* m_pbDateTime;

    // private Functions

    void createDefaultComponent();

    void takeLeftKeyAction();
    void takeRightKeyAction();
    void takeUpKeyAction();
    void takeDownKeyAction();
    void keyHandling(INST_PB_TOOLBAR_ELEMENTS_e index);
    
};

#endif // INSTANTPLAYBACK_H
