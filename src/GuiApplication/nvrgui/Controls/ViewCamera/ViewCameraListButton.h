#ifndef VIEWCAMERALISTBUTTON_H
#define VIEWCAMERALISTBUTTON_H

#include "Controls/MenuButton.h"

class ViewCameraListButton : public MenuButton
{
    Q_OBJECT
private:
    CAMERA_STATE_TYPE_e m_currentConnectionState;
    QPixmap  m_iconImage;
    QString m_imageSource;

public:
    ViewCameraListButton(int index,
                         QString label,
                         CAMERA_STATE_TYPE_e connectionState,
                         QWidget * parent = 0,
                         int indexInPage = 0,
                         bool isEnabled = true);

    void drawImage(QPainter*);
    void resetGeometry(int xOffset, int yOffset);
    void updateConnectionState(CAMERA_STATE_TYPE_e connectionState);
    void changeImage();

    void paintEvent(QPaintEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);

signals:
    void sigButtonClicked(int index,
                          CAMERA_STATE_TYPE_e connectionState = MAX_CAMERA_STATE);

protected slots:
    void slotClickEffectTimerout();
};

#endif // CAMERALISTBUTTON_H
