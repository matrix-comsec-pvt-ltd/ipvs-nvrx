#ifndef ZOOMFEATURECONTROL_H
#define ZOOMFEATURECONTROL_H

#include <QWidget>
#include "Controls/MenuButtonList.h"
#include "Controls/ImageControls/Image.h"

typedef enum
{
    ZOOM_STATE_IN,
    ZOOM_STATE_OUT
}ZOOM_STATE_TYPE_e;

typedef enum
{
    ZOOM_OUT_LABEL,
    ZOOM_EXIT_LABEL,
    MAX_ZOOM_MENU_LABEL_TYPE
}ZOOM_MENU_LABEL_TYPE_e;


class ZoomFeatureControl : public QWidget
{
    Q_OBJECT

public:
    ZoomFeatureControl(QString deviceName,
                       quint16 windowIndex,
                       QWidget* parent = 0,
                       bool isRightButtonClickAvailable = true,
                       bool isMouseMoveEventToBeForwarded = false,
                       quint16 height = 0);
    ~ZoomFeatureControl();

    bool SetViewArea(qint16 startX,
                     qint16 endX,
                     qint16 startY,
                     qint16 endY);

    void exitAction();
    void zoomOutAction();
    void changeState(ZOOM_STATE_TYPE_e zoomState);
    void mouseLeftButtonPressEvent(QMouseEvent*);
    void mouseRightButtonPressEvent(QMouseEvent*);
    void mouseLeftButtonReleaseEvent(QMouseEvent*);
    void mouseRightButtonReleaseEvent(QMouseEvent*event);

signals:
    void sigChangeLayout(LAYOUT_TYPE_e layout, DISPLAY_TYPE_e displayId,
                         quint16 windowIndex, bool ifActualWindow,
                         bool ifUpdatePage);
    void sigExitFromZoomFeature();

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void wheelEvent (QWheelEvent *event);

public slots:
    void slotMenuButtonSelected(QString menuLabel, quint8 menuIndex);
    void slotMenuListDestroyed();

private:
    ZOOM_STATE_TYPE_e m_currentZoomState;
    MenuButtonList* m_menuButtonList;
    Image* m_zoomIconImage;
    QRect m_zoomRectangle;

    QPoint m_startPoint,m_endPoint,m_refrancept,m_prevstrt;
    bool m_mouseLeftClick, m_mouseRightClick, m_isMouseMoveEventToBeForwarded;
    QStringList m_optionList;
    quint16 m_windowIndex;
    QString m_deviceName;

    QString m_zoomIconImageSource;
    Qt::MouseButton m_leftMouseButton;
    Qt::MouseButton m_rightMouseButton;
    ZOOM_MENU_LABEL_TYPE_e m_actionToPerform;

    float  m_scaleFactor;
    qint16 m_startX,m_startY,m_endX,m_endY;
    qint16 pstartx,pendx,pstarty,pendy;


    void resetCordinates();
    void updateCordinates();
};

#endif // ZOOMFEATURECONTROL_H
