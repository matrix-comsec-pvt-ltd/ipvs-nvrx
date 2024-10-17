#ifndef SYNCPLAYBACKTIMELINE_H
#define SYNCPLAYBACKTIMELINE_H

#include <QWidget>
#include <QDateTime>
#include "NavigationControl.h"
#include "Controls/ControlButton.h"
#include "Controls/TextLabel.h"
#include "Controls/Rectangle.h"
#include "Controls/SliderControl.h"
#include "Controls/SyncPlayback/SyncPlaybackRecordLine.h"
#include "Controls/SyncPlayback/SyncPlaybackToolbarButton.h"
#include "Controls/ToolTip.h"
#include "Controls/TileWithText.h"
#include "ApplController.h"

#define MAX_COL             24
#define GRID_WIDTH          SCALE_WIDTH(1500)

typedef enum
{
    SYNCPLAYBACK_TIMELINE_PREV_SLOT_BUTTON,
    SYNCPLAYBACK_TIMELINE_SLIDER_CONTROL,
    SYNCPLAYBACK_TIMELINE_NEXT_SLOT_BUTTON,
    SYNCPLAYBACK_MAX_TIMELINE_ELEMENT
}SYNCPLAYBACK_TIMELINE_ELEMENT_LIST_e;

class SyncPlaybackTimeLine : public KeyBoard, public NavigationControl
{
    Q_OBJECT
private:
    ControlButton* m_prevSlotButton;
    ControlButton* m_nextSlotButton;
    Rectangle* m_textLabelBackgroundRect;
    Rectangle* m_gridCols[MAX_COL + 1];
	SyncPlaybackRecordLine*		m_recordLine;
	Rectangle*					m_cropAndBackupIndicatorLines;
    TextLabel* m_hourIndicatorLabel[MAX_COL + 1];
    SliderControl* m_slider;
    ToolTip* m_sliderToolTip;
	BOOL	m_isSliderToolTipVisible;
    HOURS_FORMAT_TYPE_e m_currentHourFormat;
    SYNC_PLAYBACK_STATE_e m_currentPlaybackState;

    quint32 m_currentHour, m_currentMinute, m_currentSecond;
    QString m_timeString;
    quint8 m_currentSlot;
    quint32 m_cropStartingPosition, m_cropEndingPosition;

    int m_currentElement;
    NavigationControl *m_elementList[SYNCPLAYBACK_MAX_TIMELINE_ELEMENT];

    ApplController* m_applController;
public:
    SyncPlaybackTimeLine(quint16 startX,
                         quint16 startY,
                         quint16 width,
                         quint16 height,
                         quint16 indexInPage,
                         QWidget* parent = 0,
                         bool isEnable = true,
                         bool catchKey = true);
    ~SyncPlaybackTimeLine();

    void changePointToTime(int point, bool forceSec);
    int changeTimeToPoint();
    void updateTimeString();
    void changeHourFormat(HOURS_FORMAT_TYPE_e hourFormat);
    void changeCurrentSlot(quint8 currentSlot, bool changeHourIndicatorFlag = false);
    void updateTooltipPosition();
    QString changeMinuteToHour(quint32 minute);
    void showRecordsOnTimeline(quint8 iCamIndex);
    void resetRecordsOnTimeline();
    void resetCropAndBackupDataLines();
    void showCropAndBackupDataLines();
    QString getTimeString();
    void changeTime(QTime time);
    void changeState(SYNC_PLAYBACK_STATE_e state);

    void takeLeftKeyAction();
    void takeRightKeyAction();

    void forceFocusToPage(bool isFirstElement);

    virtual void navigationKeyPressed(QKeyEvent *event);
    void mouseMoveEvent(QMouseEvent *);

signals:
    void sigUpdateCurrentElement(int index);
    void sigFocusToOtherElement(bool isPrevoiusElement);
    void sigSliderPositionChanged();
    void sigSliderPositionChangedStart();

public slots:
    void slotUpdateCurrentElement(int index);
    void slotSliderValueChanged(int changedValue, int indexInpage, bool sliderMove);
    void slotControlButtonClicked(int indexInPage);
    void slotMouseReleaseOnSlider(int value, int indexInPage);
    void slotMousePressedOnSlider(int value, int indexInPage);
};

#endif // SYNCPLAYBACKTIMELINE_H
