#ifndef SYNCPLAYBACKTOOLBAR_H
#define SYNCPLAYBACKTOOLBAR_H

#include "Controls/SyncPlayback/SyncPlaybackToolbarButton.h"
#include "Controls/LayoutList.h"
#include "Controls/TextLabel.h"
#include "Controls/ToolTip.h"
#include "KeyBoard.h"
#include <QDateTime>

#define LAYOUT_LIST_BUTTON_HEIGHT		SCALE_HEIGHT(47)
#define MAX_SYNC_PB_LAYOUT				4

class SyncPlaybackToolbar : public KeyBoard, public NavigationControl
{
    Q_OBJECT
private:
    quint16 m_startX, m_startY, m_width, m_height;
    SyncPlaybackToolbarButton* m_toolbarButton[MAX_SYNCPB_BUTTON];
    TextLabel* m_dateTimeTextLabel;
    TextLabel* m_speedTextLabel;
    ToolTip* m_toolTip;
    SYNCPB_TOOLBAR_MODE_TYPE_e m_currentMode;
    LAYOUT_TYPE_e m_currentLayout;
    PB_SPEED_e m_currentSpeed;
    PB_DIRECTION_e m_currentDirection;
    LayoutList*	m_layoutList;
    int m_currentElement;
    NavigationControl* m_elementList[MAX_SYNCPB_BUTTON];

public:
    SyncPlaybackToolbar(quint16 startX,
                        quint16 startY,
                        quint16 width,
                        quint16 height,
                        int indexInPage,
                        QWidget *parent = 0);
    ~SyncPlaybackToolbar();

    void changeMode(SYNCPB_TOOLBAR_MODE_TYPE_e mode);
    void changeState(SYNC_PLAYBACK_STATE_e state);
    void changeButtonState(SYNCPB_TOOLBAR_BUTTON_TYPE_e button,
                           STATE_TYPE_e state);
    void changeButtonEnableState(SYNCPB_TOOLBAR_BUTTON_TYPE_e button,
                                bool isEnable);
    STATE_TYPE_e getButtonState(SYNCPB_TOOLBAR_BUTTON_TYPE_e button);
    bool isButtonEnable(SYNCPB_TOOLBAR_BUTTON_TYPE_e button);
    void changeDateTime(QDateTime dateTime);
    void changePlaybackSpeedDirection(PB_SPEED_e speed, PB_DIRECTION_e direction);
    void resetToolbar();
    void resetGeometry(quint16 startX,
                       quint16 startY,
                       quint16 width,
                       quint16 height);
    void takeButtonEnterKeyAction(SYNCPB_TOOLBAR_BUTTON_TYPE_e button);

    void takeLeftKeyAction();
    void takeRightKeyAction();

    void forceFocusToPage(bool isFirstElement);

    void paintEvent(QPaintEvent *);
    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void functionKeyPressed(QKeyEvent *event);
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);

	bool isLayoutListVisible(void);
	void setLayoutListVisiblity(bool iVisibleF);
signals:
    void sigFocusToOtherElement(bool isPrevoiusElement);
    void sigUpdateCurrentElement(int index);
    void sigToolbarButtonClicked(SYNCPB_TOOLBAR_BUTTON_TYPE_e index, STATE_TYPE_e state);
	void sigChangeLayout(LAYOUT_TYPE_e iLayoutType);

public slots:
    void slotButtonClicked(SYNCPB_TOOLBAR_BUTTON_TYPE_e index, STATE_TYPE_e state);
    void slotUpdateCurrentElement(int index);
    void slotShowHideTooltip(int index, bool toShowTooltip);
	void slotChangeLayout(LAYOUT_TYPE_e iLayoutType);
};

#endif // SYNCPLAYBACKTOOLBAR_H
