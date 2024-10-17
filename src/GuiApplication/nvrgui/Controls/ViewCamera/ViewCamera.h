#ifndef VIEWCAMERA_H
#define VIEWCAMERA_H

#include <QWidget>
#include "Controls/BackGround.h"
#include "../CameraList.h"

#define MAX_ELEMENTS        2

class ViewCamera : public BackGround
{
    Q_OBJECT
    CameraList   *viewCameraAddList;

    NavigationControl   *m_elementList[MAX_ELEMENTS];
    quint8               m_currentElement;

public:
    explicit ViewCamera(QWidget *parent = 0,
               quint16 windowIndex = 0);

    ~ViewCamera();

    void updateDeviceState(QString deviceName, DEVICE_STATE_TYPE_e devState);

    void takeDownKeyAction();
    void takeUpKeyAction();

    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);
    virtual void tabKeyPressed(QKeyEvent *event);
    virtual void backTab_KeyPressed(QKeyEvent *event);

    void showEvent(QShowEvent *);

signals:
    void sigSwapWindows(quint16,quint16);
    void sigStartStreamInWindow(DISPLAY_TYPE_e displayType,
                                QString deviceName,
                                quint8 channelId,
                                quint16 windowId);

public slots:
    void slotClosePage(TOOLBAR_BUTTON_TYPE_e);
    void slotSwapWindows(quint16 firstWindow,quint16 secondWindow);
    void slotUpdateCurrentElement(int);
    void slotStartStreamInWindow(DISPLAY_TYPE_e displayType,
                                 QString deviceName,
                                 quint8 channelId,
                                 quint16 windowId);
};

#endif // VIEWCAMERA_H
