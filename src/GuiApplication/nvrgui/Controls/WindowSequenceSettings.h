#ifndef WINDOWSEQUENCESETTINGS_H
#define WINDOWSEQUENCESETTINGS_H

#include <QWidget>
#include "Controls/BackGround.h"
#include "Controls/CnfgButton.h"
#include "Controls/LayoutCreator.h"
#include "Controls/SpinBox.h"
#include "Controls/Bgtile.h"
#include "Controls/OptionSelectButton.h"
#include "CameraList.h"
#include "KeyBoard.h"

typedef enum
{
    WINDOWSEQUENCE_STG_CLOSE_BUTTON,
    WINDOWSEQUENCE_STG_LAYOUTCREATOR,
    WINDOWSEQUENCE_STG_CAMERLIST,
    WINDOWSEQUENCE_STG_CHECKBOX,
    WINDOWSEQUENCE_STG_SPINBOX,
    WINDOWSEQUENCE_STG_SAVEBUTTON,
    WINDOWSEQUENCE_STG_CANCELBUTTON,

    MAX_WINDOWSEQUENCE_STG_ELEMENTS
}WINDOWSEQUENCE_STG_ELEMENTS_e;

class WindowSequenceSettings : public KeyBoard
{
    Q_OBJECT
private:
    BackGround *m_background;
    CloseButtton *m_closeButton;
    LayoutCreator *m_layoutCreator;
    BgTile *m_cameraListBgTile;
    CameraList *m_seqCameraList;
    OptionSelectButton *m_sequenceCheckbox;
    SpinBox *m_sequenceIntervalSpinbox;
    CnfgButton *m_saveButton, *m_cancelButton;
    ApplController* applController;
    DISPLAY_CONFIG_t m_currentDisplayConfig;
    DISPLAY_CONFIG_t *m_displayConfig;

    NavigationControl *m_elementList[MAX_WINDOWSEQUENCE_STG_ELEMENTS];

    quint8 m_currentElement;
    quint16 m_windowIndex;
    quint8 m_selectedChannel;
    QStringList m_sequenceIntervalList;
    bool *m_isChangeDone;


public:
    WindowSequenceSettings(quint16 windowIndex,
                           DISPLAY_CONFIG_t *displayConfig,
                           bool &isChangeDone,
                           QWidget *parent = NULL);
    ~WindowSequenceSettings();

    void saveConfig();
    void displayLayoutcreatorForCurrentConfig();
    bool findWindowIndexOfDisplayInfo(quint8 cameraIndex,
                                      QString deviceName,
                                      quint16 &windowIndex,
                                      quint8 &channelIndex);
    bool findFreeChannelIndex(quint8 &channelIndex);
    bool firstAvailableChannel(quint8 &channelIndex);
    bool isMultipleChannelAssigned(quint16 windowIndex);
    void windowCloseButtonClicked(quint8 channelIndex);

    void updateDeviceState(QString devName, DEVICE_STATE_TYPE_e devState);

    void takeLeftKeyAction();
    void takeRightKeyAction();

    void paintEvent(QPaintEvent *);
    void showEvent(QShowEvent *);

    //keyboard support added
    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);
    virtual void tabKeyPressed(QKeyEvent *event);
    virtual void backTab_KeyPressed(QKeyEvent *event);

signals:
    void sigObjectDelete();

public slots:
    void slotUpadateCurrentElement(int index);
    void slotConfigButtonClicked(int index);
    void slotClosePage(TOOLBAR_BUTTON_TYPE_e buttonIndex);
    void slotCameraButtonClicked(quint8 cameraIndex,
                                 QString deviceName,
                                 CAMERA_STATE_TYPE_e connectionState,
                                 CAMERA_STATE_TYPE_e clearConnection);
    void slotChannelSelected(quint16 index);
    void slotWindowImageClicked(WINDOW_IMAGE_TYPE_e imageType,
                                quint16 windowIndex);
    void slotWindowImageHover(WINDOW_IMAGE_TYPE_e imageType,
                              quint16 windowIndex,
                              bool isHover);
    void slotSwapWindows(quint16 firstWindow, quint16 secondWindow);
    void slotDragStartStopEvent(bool isStart);
};

#endif // WINDOWSEQUENCESETTINGS_H
