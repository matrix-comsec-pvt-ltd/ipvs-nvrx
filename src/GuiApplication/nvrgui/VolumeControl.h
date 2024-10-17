#ifndef VOLUMECONTROL_H
#define VOLUMECONTROL_H

#include <QWidget>
#include "Controls/BackGround.h"
#include "Controls/TextLabel.h"
#include "Controls/OptionSelectButton.h"
#include "ApplController.h"
#include "Controls/SliderControl.h"

typedef enum
{
    MUTE_STATUS,
    UNMUTE_STATUS,
    MAX_MUTE_STATUS
}AUDIO_STATUS_e;

typedef enum
{
    VOLUME_SLIDER = 0,
    MUTE_BUTTON,
    MAX_VOLUME_CONTROL_ELEMENT
}VOLUME_CONTROL_PAGE_ELEMENT_e;

class VolumeControl : public BackGround
{
    Q_OBJECT
private:
    QPoint m_lastPoint;
    bool m_mousePressedFlag;
    int m_audioLevel, m_sliderImageType, m_currentMuteState,m_chngMuteState;
    Rectangle* m_topBorder;
    Rectangle* m_rightBorder;
    Rectangle* m_rightMostBorder;
    Rectangle* m_volumeSliderBackGround;
    TextLabel* m_headingText;
    OptionSelectButton* m_muteOption;
    SliderControl* m_volumeSlider;
    float m_valueMultipler;

    ApplController* m_applController;

    NavigationControl* m_elementList[MAX_VOLUME_CONTROL_ELEMENT];
    int m_currentElement;

public:
    VolumeControl(int startX, int startY, int width, int height, QWidget *parent = 0);
    ~VolumeControl();

    void readAudioConfig();
    void writeAudioConfig(int currentState, int audioLevel);

    void takeMuteKeyAction();
    void takeLeftKeyAction();
    void takeRightKeyAction();

    //keyboard methods
    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);

signals:
    void sigChangeToolbarButtonState(TOOLBAR_BUTTON_TYPE_e buttonIndex, STATE_TYPE_e state);

public slots:
    void slotMuteButtonClicked(OPTION_STATE_TYPE_e currentState, int indexInPage);
    void slotValueChanged(int changedValue, int indexInPage, bool sliderMove);
    void slotUpdateCurrentElement(int index);
};

#endif // VOLUMECONTROL_H
