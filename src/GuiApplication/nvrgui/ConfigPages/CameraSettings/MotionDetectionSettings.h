#ifndef MOTIONDETECTIONSETTINGS_H
#define MOTIONDETECTIONSETTINGS_H

#include <QWidget>
#include "DataStructure.h"
#include "Controls/TextLabel.h"
#include "Controls/InfoPage.h"
#include "Controls/MenuButtonList.h"
#include "ApplController.h"
#include "Controls/PickListLoader.h"

typedef enum
{
    MOTIONDETECTION_CLEAR_LABEL,
    MOTIONDETECTION_EXIT_LABEL,
    MAX_MOTIONDETECTION_MENU_LABEL_TYPE
}MOTIONDETECTION_MENU_LABEL_TYPE_e;

class MotionDetectionSettings : public QWidget
{
    Q_OBJECT

private:

    QWidget*                             m_inVisibleWidget;
    InfoPage*                            m_infoPage;
    MenuButtonList*                      m_menuButtonList;
    ApplController*                      m_applController;
    PickListLoader*                      m_picklistLoader;
    MOTION_DETECTION_WINDOWINFO_t*       m_motionDetectionData;


    VIDEO_STANDARD_e                     m_videoStandard;
    MOTION_MASK_RECTANGLE_t              m_rectInfo[MAX_MOTIONDETECTION_AREA];
    MOTIONDETECTION_MENU_LABEL_TYPE_e    m_actionToPerform;

    bool                                 m_mouseLeftClick, m_mouseRightClick;
    qreal                                m_blockWidth, m_blockHeight;
    quint8                               m_currentRectNumber;
    quint8                               m_hightlightedRectNumber;
    quint8                               m_currentEditRectNumber;
    quint8                               m_textWidth;
    quint8                               m_sensitivityTextWidth, m_textHeight;
    quint8                               m_startCol, m_startRow;
    QPoint                               m_startPoint, m_rightClickPoint;
    quint16                              m_maxRow;
    QStringList                          m_sensitivityLevelList;

    QRect                  m_maskRectangle[MAX_MOTIONDETECTION_AREA];
    QRect                  m_maskTextRectangle[MAX_MOTIONDETECTION_AREA];
    QRect                  m_maskSensitivityRectangle[MAX_MOTIONDETECTION_AREA];

public:
    MotionDetectionSettings(void *data,QString deviceName, QWidget* parent = 0);
    ~MotionDetectionSettings();

    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);

public slots:
    void slotMenuButtonSelected(QString menuLabel, quint8 menuIndex);
    void slotMenuListDestroyed();
    void slotInfoPageButtonClicked(int index);
    void slotValueChanged(quint8 key, QString value, bool);
    void slotPickListDestroyed();

private:

    void createMaskRectangle(quint8 rectIndex);
    void setGeometryForElements();

    quint8 findRectangleNumber();
    quint8 findRowOfPoint(QPoint point);
    quint8 findColOfPoint(QPoint point);

    bool isMaskRectContainsPoint(QPoint point);
    bool isMaskRectContainsPoint(QPoint point, quint8 rectIndex);
    bool isRectangleOverlapping(quint8 rectIndex);

    void loadPickList();
    void clearRect();
    void exitAction();
    void saveConfig();

    void mouseLeftButtonPressEvent(QMouseEvent*);
    void mouseRightButtonPressEvent(QMouseEvent*);
    void mouseLeftButtonReleaseEvent(QMouseEvent*);
    void mouseRightButtonReleaseEvent(QMouseEvent*);

    void forceActiveFocus();

};

#endif // MOTIONDETECTIONSETTINGS_H
