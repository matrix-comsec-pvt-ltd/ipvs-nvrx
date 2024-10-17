#ifndef DEVICELISTBUTTON_H
#define DEVICELISTBUTTON_H

#include "MenuButton.h"
#include "Controls/ImageControls/Image.h"

#define CAMERA_LIST_BUTTON_WIDTH        SCALE_WIDTH(260)
#define CAMERA_LIST_BUTTON_HEIGHT       SCALE_HEIGHT(30)

typedef enum
{
    DEV_DESELECTED,
    DEV_SELECTED,

    MAX_DEV_SEL_STATE
}DEV_SELECTION_e;

class DeviceListButton : public MenuButton
{
    Q_OBJECT
private:
    DEVICE_STATE_TYPE_e m_currentConnectionState;
    QPixmap  m_iconImage;
    QString m_imageSource;
    bool m_stateChangeImgEnable;

    Image *m_devStatusChangeImg;
public:
    DeviceListButton(int index,
                     QString label,
                     DEVICE_STATE_TYPE_e connectionState,
                     QWidget * parent = 0,
                     int indexInPage = 0,
                     bool isEnabled = true,
                     bool stateChangeImgEnable = false,
                     quint16  deviceWidth = CAMERA_LIST_BUTTON_WIDTH);
    ~DeviceListButton();

    void drawImage(QPainter*);
    void resetGeometry(int xOffset, int yOffset);
    void resetGeometryCustIndex(int , int );
    void updateConnectionState(DEVICE_STATE_TYPE_e connectionState);
    DEVICE_STATE_TYPE_e getConnectionstate();
    void changeImage();
    void changeDevSelectionstate(DEV_SELECTION_e state);

    void paintEvent(QPaintEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void enterKeyPressed(QKeyEvent *event);

signals:
    void sigDevStateChangeImgClick(int index,
                                   DEVICE_STATE_TYPE_e connectionState);

public slots:
    void slotDevStatusChangeImgClick(int);
};

#endif // DEVICELISTBUTTON_H
