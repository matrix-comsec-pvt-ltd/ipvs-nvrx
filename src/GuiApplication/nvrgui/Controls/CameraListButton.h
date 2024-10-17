#ifndef CAMERALISTBUTTON_H
#define CAMERALISTBUTTON_H

#include "MenuButton.h"

#define CONNECT_ALL_STR         "Connect All"
#define DISCONNECT_ALL_STR      "Disconnect All"

typedef enum
{
    CALLED_BY_DISPLAY_SETTING,
    CALLED_BY_WINDOWSEQ_SETTING,
    CALLED_BY_VIEWCAM_ADD_LIST,
    MAX_CALLED_BY_CLASS
} CAMERALIST_CALLED_BY_e;

class CameraListButton : public MenuButton
{
    Q_OBJECT
private:
    CAMERA_STATE_TYPE_e m_currentConnectionState;
    QPixmap  m_iconImage;
    QString m_imageSource;
    QString m_connectStr;
    CAMERALIST_CALLED_BY_e m_createdBy;

public:
    CameraListButton(int index,
                     QString label,
                     CAMERA_STATE_TYPE_e connectionState,
                     QWidget * parent = 0,
                     int indexInPage = 0,
                     bool isEnabled = true,
                     int deviceIndex = -1,
                     CAMERALIST_CALLED_BY_e createdBy  = MAX_CALLED_BY_CLASS);

    void drawImage(QPainter*);
    void resetGeometry(int xOffset, int yOffset);
    void resetGeometryCustIndex(int ,int);
    void updateConnectionState(CAMERA_STATE_TYPE_e connectionState);
    void changeImage();
    void changeText(QString newStr);
    CAMERA_STATE_TYPE_e getConnectionState();

    void paintEvent(QPaintEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    int getDeviceIndex();

signals:
    void sigButtonClicked(int index,
                          CAMERA_STATE_TYPE_e connectionState = MAX_CAMERA_STATE,int deviceIndex= -1);

protected slots:
    void slotClickEffectTimerout();
};

#endif // CAMERALISTBUTTON_H
