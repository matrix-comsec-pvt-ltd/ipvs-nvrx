#ifndef LAYOUTCREATOR_H
#define LAYOUTCREATOR_H

#include <QWidget>
#include <QGridLayout>
#include <QOpenGLWidget>
#include "NavigationControl.h"
#include "Controls/LayoutWindow.h"
#include "Controls/ProxyLayoutWindow.h"

#define MAX_PROXY_WINDOW    64

const quint8 windowPerPage[MAX_LAYOUT] =
{
    1,  // layout 1X1
    4,  // layout 2x2
    6,  // layout 1+5
    7,  // layout 3+4
    8,  // layout 1+7
    9,  // layout 3x3
    10, // layout 2+8
    10, // layout 1+9
    13, // layout 1+12
    13, // layout 1c+12
    13, // layout 4+9
    14, // layout 2+12
    16, // layout 4x4
    25, // layout 5x5
    36, // layout 6x6
    64, // layout 8x8
    1,  // layout 1X1 Playback
    4,  // layout 2X2 Playback
    9,  // layout 3X3 Playback
    16  // layout 4X4 Playback
};

class LayoutCreator : public KeyBoard, public NavigationControl
{
    Q_OBJECT
private:
    LayoutWindow* m_windows[MAX_WINDOWS];
    ProxyLayoutWindow* m_proxyWindow[MAX_PROXY_WINDOW];
    QGridLayout* m_layoutBg;

    quint16 m_selectedWindow;
    quint8 m_proxyWindowCount;
    LAYOUT_TYPE_e m_selectedLayout;

    quint16 m_firstWindowIndex, m_secondWindowIndex;
    bool m_isDragEvent;

    quint16 m_maxWindows;
    WINDOW_TYPE_e m_windowType;

public:
    LayoutCreator(quint32 startx,
                  quint32 starty,
                  quint32 width,
                  quint32 height,
                  QWidget *parent = 0,
                  int indexInPage  = 0,
                  WINDOW_TYPE_e windowType = WINDOW_TYPE_LAYOUT,
                  bool isEnabled = true,
                  quint16 maxWindows = MAX_CHANNEL_FOR_SEQ);
    ~LayoutCreator();

    void setLayoutStyle(LAYOUT_TYPE_e newLayout,
                        quint16 newSelectedWindow);
    void updateWindowData(quint16 windowIndex);

    void updateWindowHeaderText(quint16 windowIndex,QString updateWindowHeader);

    void setWinHeaderForDispSetting(quint16 windowIndex, QString camName);
    void getWindowDimensionInfo(quint16 windowIndex, WIN_DIMENSION_INFO_t *info);
    quint8 getPbtoolbarSize(quint16 windowIndex);
    WINDOW_ICON_TYPE_e getWindowIconType(quint16 windowIndex);
    void changeSelectedWindow(quint16 windowIndex, bool isFocusToWindow = false);
    void giveFocusToWindow();
    qint8 findNeighbourOfWindow(WIN_NEIGHBOUR_TYPE_e neighbourType);
    bool naviagtionInLayoutCreator(WIN_NEIGHBOUR_TYPE_e neighbourType);
    void changeWindowType(quint16 windowIndex, WINDOW_TYPE_e windowType);
    void updateImageMouseHover(WINDOW_IMAGE_TYPE_e imageType, quint16 windowIndex, bool isHover);
    void updateSequenceImageType(quint16 windowIndex, WINDOWSEQUENCE_IMAGE_TYPE_e configImageType);
    void setMaxWinodws(quint16 maxWind);
    bool isNextPageAllProxyWindow();

    void forceFocusToPage(bool isFirstElement);

    virtual void navigationKeyPressed(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    QWidget* window(quint16 windowIndex);

    void updateApToolbar(quint16 windowIndex);
    void updateAPNextVideoText(quint16 windowIndex);
signals:
    void sigWindowSelected(quint16 windowIndex);
    void sigLoadMenuListOptions(quint16 windowIndex);
    void sigWindowDoubleClicked(quint16 windowIndex);
    void sigEnterKeyPressed(quint16);
    void sigWindowImageClicked(WINDOW_IMAGE_TYPE_e imageType, quint16 windowIndex);
    void sigWindowImageHover(WINDOW_IMAGE_TYPE_e imageType, quint16 windowIndex, bool isHover);

    void sigSwapWindows(quint16 firstWindow, quint16 secondWindow);
    void sigDragStartStopEvent(bool isStart);
    void sigFocusToOtherElement(bool isPrevoiusElement);
    void sigChangePageFromLayout();
    void sigUpdateCurrentElement(int index);
    void sigAPCenterBtnClicked(quint8 index, quint16 windowId);
    void sigStopAsyncPBTimer();

public slots:
    void slotWindowSelected(quint8 windowIndex);
    void slotLoadMenuListOptions(quint8 windowIndex);
    void slotWindowDoubleClicked(quint8 windowIndex);
    void slotEnterKeyPressed(quint8 windowIndex);
    void slotWindowImageClicked(WINDOW_IMAGE_TYPE_e imageType, quint8 windowIndex);
    void slotWindowImageHover(WINDOW_IMAGE_TYPE_e imageType, quint8 windowIndex, bool isHover);
    void slotSwapWindow(quint8 firstWindow, quint8 secondWindow);

    void slotAPCenterBtnClicked(quint8 index, quint16 windowId);
};

#endif // LAYOUTCREATOR_H
