#ifndef PRIVACYMASKSETTINGS_H
#define PRIVACYMASKSETTINGS_H

#include <QWidget>
#include "DataStructure.h"
#include "Controls/TextLabel.h"
#include "Controls/InfoPage.h"
#include "Controls/MenuButtonList.h"
#include "ApplController.h"
#include "PayloadLib.h"

typedef struct
{
    quint16 startX;
    quint16 startY;
    quint16 endX;
    quint16 endY;
    quint8 index;
    quint8 creatingOrder;
}PRIVACY_MASK_RECTANGLE_t;

typedef enum
{
    PRIVACYMASK_CLEAR_LABEL,
    PRIVACYMASK_EXIT_LABEL,
    MAX_PRIVACYMASK_MENU_LABEL_TYPE
}PRIVACYMASK_MENU_LABEL_TYPE_e;

class PrivacyMaskSettings : public QWidget
{
    Q_OBJECT
private:
    VIDEO_STANDARD_e m_videoStandard;
    PRIVACY_MASK_DATA_t *m_privacyMaskData;
    PRIVACY_MASK_RECTANGLE_t m_rectInfo[MAX_PRIVACYMASK_AREA];
    PRIVACYMASK_MENU_LABEL_TYPE_e m_actionToPerform;

    InfoPage* m_infoPage;
    MenuButtonList* m_menuButtonList;
    ApplController* m_applController;

    QPoint m_startPoint, m_rightClickPoint;
    bool m_mouseLeftClick, m_mouseRightClick, m_drawRectFlag;
    quint16 m_maxHeight, m_heightRatio;
    quint16 m_widthRatio;
    quint8 m_currentRectNumber, m_hightlightedRectNumber, m_currentTotalRect;
    QRect m_maskRectangle[MAX_PRIVACYMASK_AREA];
    QRect m_maskTextRectangle[MAX_PRIVACYMASK_AREA];
    quint8 m_textWidth, m_textHeight;    
    quint8 m_maxSupportedPrivacyMaskWindow;

    QString m_deviceName;
    quint8 m_cameraIndex;
    CAMERA_TYPE_e m_currentCamera;
    PayloadLib* m_payloadLib;

public:
    PrivacyMaskSettings(void *data,
                        QString deviceName,
                        quint8 cameraIndex,
                        CAMERA_TYPE_e cameraType,
                        quint8  maxSupportedPrivacyMaskWindow,
                        QWidget* parent = 0);

    ~PrivacyMaskSettings();

    void setGeometryForElements();
    void createMaskRectangle(quint8 rectIndex);
    quint8 findRectangleNumber();
    void resizeRectangle();    
    bool isMaskRectContainsPoint(QPoint point, quint8 &rectIndex);
    void exitAction();
    void clearRect();
    bool isMaskRectangleBeyondLimit();
    void mouseLeftButtonPressEvent(QMouseEvent*);
    void mouseRightButtonPressEvent(QMouseEvent*);
    void mouseLeftButtonReleaseEvent(QMouseEvent*);
    void mouseRightButtonReleaseEvent(QMouseEvent*);

    void saveConfig();

    void forceActiveFocus();

    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);

signals:
    void sigLoadProcessBar();

public slots:
    void slotMenuButtonSelected(QString menuLabel, quint8 menuIndex);
    void slotMenuListDestroyed();
    void slotInfoPageButtonClicked(int index);
};

#endif // PRIVACYMASKSETTINGS_H
