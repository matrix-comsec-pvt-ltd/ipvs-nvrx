#ifndef IPMOTIONDETECTIONSETTINGS_H
#define IPMOTIONDETECTIONSETTINGS_H

#include <QWidget>
#include "DataStructure.h"
#include "Controls/TextLabel.h"
#include "Controls/InfoPage.h"
#include "Controls/MenuButtonList.h"
#include "ApplController.h"
#include "PayloadLib.h"
#include "Controls/PickListLoader.h"
#include "Controls/TextBox.h"
#include "Controls/Heading.h"
#include "Controls/Closebuttton.h"

#define MAX_MOTION_BLOCK							1584

typedef enum
{
    IPMOTIONDETECTION_CLEAR_LABEL,
    IPMOTIONDETECTION_EXIT_LABEL,
    MAX_IPMOTIONDETECTION_MENU_LABEL_TYPE
}IPMOTIONDETECTION_MENU_LABEL_TYPE_e;

typedef enum
{
    IPMOTIONDETECTION_IP_POINT_EVENT_LABEL,
    IPMOTIONDETECTION_IP_POINT_SENSITIVITY_LABEL,
    IPMOTIONDETECTION_IP_POINT_NO_MOTION_DURATION_LABEL,
    IPMOTIONDETECTION_IP_POINT_CLEAR_ALL_LABEL,
    IPMOTIONDETECTION_IP_POINT_EXIT_LABEL,
    MAX_IPMOTIONDETECTION_IP_POINT_MENU_LABEL_TYPE
}IPMOTIONDETECTION_IP_POINT_MENU_LABEL_TYPE_e;

class IpMotionDetectionSettings : public QWidget
{
    Q_OBJECT

private:
    InfoPage*                                       m_infoPage;
    MenuButtonList*                                 m_menuButtonList;
    PickListLoader*                                 m_picklistLoader;
    TextBox*                                        m_NoMotionDurationTextBox;
    TextboxParam*                                   m_NoMotionDurationTextBoxParam;
    QWidget*                                        m_inVisibleWidget;
    ApplController*                                 m_applController;
    PayloadLib*                                     m_payloadLib;
    MOTION_DETECTION_CONFIG_t*                      m_motionDetectionConfig;
    Rectangle*                                      m_background;
    CloseButtton*                                   m_closeButton;
    Heading*                                        m_pageHeading;

    bool                                            m_mouseLeftClick, m_mouseRightClick;
    bool                                            m_isOverlapingAllowed;
    QString                                         m_deviceName;
    QStringList                                     m_sensitivityLevelList;
    QRect                                           m_maskRectangle[MAX_MOTION_BLOCK];
    QRect                                           m_maskTextRectangle[MAX_MOTION_BLOCK];
    QRect                                           m_maskSensitivityRectangle[MAX_MOTION_BLOCK];
    qreal                                           m_blockWidth, m_blockHeight;
    QPoint                                          m_startPoint, m_rightClickPoint;

    quint8                                          m_noMotionEventSupportF;
    quint8                                          m_isNoMotionEvent;
    quint8                                          m_startCol, m_startRow;
    quint8                                          m_sensitivity;
    quint8                                          m_cameraIndex;
    quint16                                         m_noMotionDuration;
    quint16                                         m_maxRow;
    quint16                                         m_currentRectNumber, m_hightlightedRectNumber;
    quint16                                         m_currentEditRectNumber;
    quint16                                         m_maxMotionBlock;
    quint16                                         m_maxHeight, m_heightRatio;
    quint16                                         m_textWidth, m_sensitivityTextWidth, m_textHeight;

    MOTION_MASK_RECTANGLE_t                         m_rectInfo[MAX_MOTION_BLOCK];
    MOTION_DETECTION_SUPPORT_TYPE_e                 m_motionSupportType;
    IPMOTIONDETECTION_MENU_LABEL_TYPE_e             m_actionToPerform;
    IPMOTIONDETECTION_IP_POINT_MENU_LABEL_TYPE_e    m_pointActionToPerform;

public:
    IpMotionDetectionSettings(void *data, QString deviceName, quint8 cameraIndex, QWidget* parent = 0);
    ~IpMotionDetectionSettings();

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
    void slotValueChanged(quint8 key, QString value, bool);
    void slotPickListDestroyed();
    void slotCloseButtonClicked(int index);

private:
    quint8 findRowOfPoint(QPoint point);
    quint8 findColOfPoint(QPoint point);
    quint16 findRectangleNumber();

    bool isMaskRectContainsPoint(QPoint point);
    bool isMaskRectContainsPoint(QPoint point, quint16 rectIndex);
    bool isRectangleOverlapping(quint16 rectIndex);

    void setGeometryForElements();
    void createMaskRectangle(quint16 rectIndex);

    void saveConfig();
    void exitAction();
    void clearRect();
    void clearAllRect();
    void loadPickList();
    void forceActiveFocus();

    void mouseLeftButtonPressEvent(QMouseEvent*);
    void mouseRightButtonPressEvent(QMouseEvent*);
    void mouseLeftButtonReleaseEvent(QMouseEvent*);
    void mouseRightButtonReleaseEvent(QMouseEvent*);    
};

#endif // IPMOTIONDETECTIONSETTINGS_H
