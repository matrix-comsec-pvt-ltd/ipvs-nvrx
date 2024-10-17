#ifndef SYNCPLAYBACKTOOLBARBUTTON_H
#define SYNCPLAYBACKTOOLBARBUTTON_H

#include <QWidget>
#include "ApplController.h"
#include "EnumFile.h"
#include "NavigationControl.h"
#include "KeyBoard.h"
#include <QTimer>

typedef enum
{
    PLAY_BUTTON,
    STOP_BUTTON,
    REVERSE_PLAY_BUTTON,
    SLOW_PLAY_BUTTON,
    FAST_PLAY_BUTTON,
    PREVIOUS_FRAME_BUTTON,
    NEXT_FRAME_BUTTON,
    AUDIO_BUTTON,
    ZOOM_BUTTON,
    CROP_AND_BACKUP_BUTTON,
    LIST_BUTTON,
    LAYOUT_BUTTON,
    CHANGE_MODE_BUTTON,
    MAX_SYNCPB_BUTTON
}SYNCPB_TOOLBAR_BUTTON_TYPE_e;

typedef enum
{
    NORMAL_MODE,
    FULL_MODE,
    MAX_SYNCPB_TOOLBAR_MODE_TYPE
}SYNCPB_TOOLBAR_MODE_TYPE_e;

class SyncPlaybackToolbarButton : public KeyBoard, public NavigationControl
{
    Q_OBJECT

private:
    SYNCPB_TOOLBAR_BUTTON_TYPE_e m_index;
    SYNCPB_TOOLBAR_MODE_TYPE_e m_currentMode;
    QString m_imageSource;
    STATE_TYPE_e m_currentState;
    IMAGE_TYPE_e m_currentImageType;
    bool m_isStateAvailable;
    QPixmap m_buttonImage;

    QTimer* m_clickEffectTimer;

public:
    SyncPlaybackToolbarButton(SYNCPB_TOOLBAR_BUTTON_TYPE_e index,
                              QWidget * parent,
                              int indexInPage = 0,
                              bool isEnable = true);
    ~SyncPlaybackToolbarButton();

    void changeButtonImage(IMAGE_TYPE_e type);
    void changeButtonState(STATE_TYPE_e state);
    void changeMode(SYNCPB_TOOLBAR_MODE_TYPE_e mode);
    STATE_TYPE_e getButtonState();
    void selectControl();
    void deSelectControl();

    void forceActiveFocus();
    void setIsEnabled(bool isEnable);

    void takeEnterKeyAction();

    //virtual functions inherited from QWidget
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent * event);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    virtual void enterKeyPressed(QKeyEvent *event);

signals:
    void sigButtonClicked(SYNCPB_TOOLBAR_BUTTON_TYPE_e index, STATE_TYPE_e state);
    void sigUpdateCurrentElement(int index);
    void sigShowHideToolTip(int index, bool toShowTooltip);

public slots:
    void slotClickEffectTimeOut();
};

#endif // SYNCPLAYBACKTOOLBARBUTTON_H
