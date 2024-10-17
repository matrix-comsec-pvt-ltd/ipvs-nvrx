#ifndef LIVEVIEWTOOLBAR_H
#define LIVEVIEWTOOLBAR_H

#include "Controls/LiveViewToolbar/LiveViewToolbarButton.h"
#include "Controls/TextLabel.h"
#include "Controls/ToolTip.h"
#include "Controls/Closebuttton.h"
#include "ApplController.h"
#include "KeyBoard.h"
#include <QTimer>

class LiveViewToolbar : public KeyBoard
{
    Q_OBJECT
private:
    quint16 m_startX, m_startY, m_width, m_height;
    WINDOW_ICON_TYPE_e m_windowIconType;
    quint16 m_windowIndex;
    quint8 m_totalButtons;
    LiveViewToolbarButton *m_toolbarButton[MAX_LIVEVIEW_BUTTON];
    ToolTip *m_toolTip;
    QTimer *m_keepAliveTimer;
    bool isUnloadNeeded;

    int m_currentElement;
    NavigationControl *m_elementList[MAX_LIVEVIEW_BUTTON];

public:
    LiveViewToolbar(quint16 startX,
                    quint16 startY,
                    quint16 width,
                    quint16 height,
                    quint16 windowIndex,
                    WINDOW_ICON_TYPE_e windowIconType,
                    QWidget *parent = 0);
    ~LiveViewToolbar();

    void changeButtonState(LIVEVIEW_TOOLBAR_BUTTON_TYPE_e button,
                           STATE_TYPE_e state);

    void takeLeftKeyAction();
    void takeRightKeyAction();
    quint16 getWindowIndex() const;

    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);

    void mouseMoveEvent (QMouseEvent *);
    bool eventFilter (QObject * obj, QEvent *event);

signals:
    void sigToolbarButtonClicked(LIVEVIEW_TOOLBAR_BUTTON_TYPE_e index,
                                 STATE_TYPE_e state,
                                 quint16 windowIndex);
    void sigCloseLiveViewToolbar();

public slots:
    void slotButtonClicked(LIVEVIEW_TOOLBAR_BUTTON_TYPE_e index,
                           STATE_TYPE_e state);
    void slotUpdateCurrentElement(int index);
    void slotShowHideTooltip(int index, bool toShowTooltip);
    void slotKeepAliveTimeOut();
};
#endif // LIVEVIEWTOOLBAR_H
