#ifndef LAYOUTWINDOW_H
#define LAYOUTWINDOW_H

#include <QWidget>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>

#include "EnumFile.h"
#include "Controls/ImageControls/WindowIcon.h"
#include "Controls/TextWithBackground.h"
#include "LayoutWindowRectangle.h"
#include "Controls/ImageControls/Image.h"
#include "NavigationControl.h"
#include "DataStructure.h"
#include "ToolTip.h"
#include "Controls/TextLabel.h"
#include "KeyBoard.h"

#define MAX_AP_TIMER_COUNT              5

typedef enum
{
    CAMERA_STATUS_ICON,
    RECORDING_STATUS_ICON,
    MOTION_DETECTION_ICON,
    VIEW_TEMPERING_ICON,
    OBJECT_INTRUSION_ICON,
    TRIP_WIRE_ICON,
    AUDIO_EXCEPTION_ICON,
    AUDIO_ON_ICON,
	MICROPHONE_STATUS_ICON,
    MISSING_ICON,
    SUSPICIOUS_ICON,
    LOITRING_ICON,
    MAX_WINDOW_ICON
}LAYOUT_WINDOWICON_TYPE_e;

typedef enum
{
    WINDOW_TYPE_LAYOUT,
    WINDOW_TYPE_DISPLAYSETTINGS,
    WINDOW_TYPE_SYNCPLAYBACK,
    WINDOW_TYPE_SEQUENCESETTINGS,
    WINDOW_TYPE_VIDEO_POP_UP,
    WINDOW_TYPE_LOCAL_DECORDING
}WINDOW_TYPE_e;

typedef enum
{
    WINDOW_CLOSE_BUTTON,
    WINDOW_AUDIO_ON_BUTTON,
    WINDOW_ADD_CAMERA_BUTTON,
    WINDOW_SEQUENCE_BUTTON,
    WINDOW_OPEN_TOOLBAR_BUTTON
}WINDOW_IMAGE_TYPE_e;

typedef enum
{
    MULTIPLE_CHANNEL_ASSIGN,
    NONE_CHANNEL_ASSIGN,
    NO_WINDOWSEQUENCE_IMAGE
}WINDOWSEQUENCE_IMAGE_TYPE_e;

typedef enum
{
    AP_TOOLBAR_RELOAD,
    AP_TOOLBAR_PREVIOUS,
    AP_TOOLBAR_NEXT,
    MAX_AP_TOOLBAR_BTN
}AP_TOOLBAR_BTN_e;

typedef enum
{
    SMALL_CENTER_ICON,
    LARGE_CENTER_ICON,
    MAX_CENTER_ICON
}CENTER_IMAGE_SIZE_e;

class LayoutWindow : public KeyBoard, public NavigationControl, protected QOpenGLFunctions
{
    Q_OBJECT
private:
    TextWithBackground* m_windowHeaderText;
    TextLabel* m_windowNumberText;
    Image* m_centerWindowIcon;
    Image* m_pbIcon;
    Image* m_audioOnIcon;
    WindowIcon* m_windowIcons[MAX_WINDOW_ICON];
    Image* m_closeButtonIcon;
    Image* m_sequenceIcon;
    Image* m_openToolbarIcon;
	LayoutWindowRectangle* m_borderRect;
    ToolTip*   m_toolTip;
    ToolTip*   m_windowHeaderToolTip;
    ToolTip*   m_windowToolTip;
    ToolTip*   m_decoderTooltip;

    WINDOW_TYPE_e m_windowType;
    WINDOWSEQUENCE_IMAGE_TYPE_e m_sequenceImgType;
    DEV_CAM_INFO_t m_camInfo;
    WINDOW_ICON_TYPE_e m_windowIconType;
    ApplController* applController;
    quint8 m_apTimerCount;
    quint8 m_recordingHealthStatus;
    quint8 m_motionDetectionHealtStatus;
    quint8 m_viewTemperingHealthStatus;
    quint8 m_objectIntrusionHealthStatus;
    quint8 m_tripWireHealthStatus;
    quint8 m_audioExceptionHealthStatus;
    quint8 m_missingObjectStatus;
    quint8 m_suspiousObjectStatus;
    quint8 m_loiteringStatus;
    quint8 m_objectcountingStatus;
    bool   m_audioOn;
	bool	m_microPhoneStatus;
    quint8 m_windowIndex;
    quint8 m_pbToolbarSize;
    quint8 m_apToolbarSize;
    QString m_windowColor;
    QString m_windowIconImageSource[MAX_WINDOW_ICON];
    QString m_windowIconPath, m_pbIconPath;
    QString m_pbSpeedStr;
    QString m_pbdateTimeStr;
    quint16 m_arrayIndex;

    Image*     m_apReloadIcon;
    Image*     m_apPrevIcon;
    Image*     m_apNextIcon;
    TextLabel* m_apNextVideoText;
    Rectangle* m_apNextTile;
    QString    m_centerImageSizePath;
	BOOL		m_ActivateCenterWindowIconHoverF;

public:
    explicit LayoutWindow(quint8 index,
                 QWidget *parent = 0,
                 WINDOW_TYPE_e windowType = WINDOW_TYPE_LAYOUT,
                 bool isEnabled = true,
                 bool catchKey = true);
    ~LayoutWindow();

    void setWindowIconType(WINDOW_ICON_TYPE_e iconType);
    void updateWindowHeaderText(QString headerText);
    void updateWindowData(quint16 arrayWindowIndex);
    void updateCameraStatusOSD();
    void updateWindowHeaderOSD();
    void updateCamInfoOnWindow(quint16 arrayWindowIndex);
    void clearWindowIconImageSources(quint8 iconIndex = 0);
    void changeWindowColor(QString color);
    void selectWindow();
    void deselectWindow();
    void clearWindow(quint16 arrayWindowIndex);
    void clearWindowData();
    void clearPlaybackRelatedInfo(quint16 arrayWindowIndex);
    quint8 getWindowIndex();
    void changeWindowType(WINDOW_TYPE_e windowType);
    WINDOW_TYPE_e getWindowType();
    void raiseWindowIcons();

    void setWinHeaderTextForDispSetting(QString camName);
    void setWindowNumber(quint16 index);

    void setPbtoolbarSize(quint8 size);
    quint8 getPbToolbarSize();
    WINDOW_ICON_TYPE_e getWindowIconType();

    void updateImageMouseHover(WINDOW_IMAGE_TYPE_e imageType, bool isHover);
    void updateSequenceImageType(WINDOWSEQUENCE_IMAGE_TYPE_e configImageType);

    void forceActiveFocus();
    void setIsEnabled(bool isEnable);

    void takeEnterKeyAction();
    void takeMenuKeyAction();
    void takeCancelKeyAction();
    static quint8 m_firstClickedWindow;
    static quint8 getFirstClickedWindow();

    void updateWindowHeaderForVideoPopup(quint16 windowIndex);
    quint16 m_arrayOfIndex;
    QString HeaderTextForVideoPopup(QString header , quint8 maxValue);

    void showAPToolbar(bool isApToolbarVisible, bool isPrevEnable, bool isNextEnable);
    void updateApToolbar(quint16 windowIndex);
    void setAPToolbarSize(quint8 size);

    bool getApFeatureStatus();
    void updateAPNextVideoText(quint16 windowIndex);

    //keyboard method
    virtual void enterKeyPressed(QKeyEvent *event);
    virtual void insertKeyPressed(QKeyEvent *event);
    virtual void deleteKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    bool eventFilter(QObject *, QEvent *);
    void resizeEvent(QResizeEvent *);

signals:
    void sigWindowSelected(quint8 windowIndex);
    void sigLoadMenuListOptions(quint8 windowIndex);
    void sigWindowDoubleClicked(quint8 windowIndex);
    void sigEnterKeyPressed(quint8 windowIndex);
    void sigWindowImageClicked(WINDOW_IMAGE_TYPE_e imageType, quint8 windowIndex);
    void sigWindowImageHover(WINDOW_IMAGE_TYPE_e imageType, quint8 windowIndex, bool isHover);
    void sigSwapWindow(quint8,quint8);
    void sigAPCenterBtnClicked(quint8 index,quint16 windowIndex);

public slots:
    void slotImageClicked(int indexInPage);
    void slotMouseHover(int index, bool state);
    void slotAPToolbarBtnClicked(int index);
};

#endif // LAYOUTWINDOW_H
