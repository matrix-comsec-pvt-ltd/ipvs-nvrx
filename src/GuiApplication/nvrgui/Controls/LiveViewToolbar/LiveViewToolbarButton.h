#ifndef LIVEVIEWTOOLBARBUTTON_H
#define LIVEVIEWTOOLBARBUTTON_H

#include <QWidget>
#include "ApplController.h"
#include "EnumFile.h"
#include "NavigationControl.h"
#include "KeyBoard.h"
#include <QTimer>

typedef enum
{
    LIVEVIEW_STREAMTYPE_BUTTON,
    LIVEVIEW_MENUSETTINGS_BUTTON,
    LIVEVIEW_PTZCONTROL_BUTTON,
    LIVEVIEW_SNAPSHOT_BUTTON,
    LIVEVIEW_ZOOM_BUTTON,
    LIVEVIEW_RECORDING_BUTTON,
    LIVEVIEW_AUDIO_BUTTON,
	LIVEVIEW_MICROPHONE_BUTTON,
    LIVEVIEW_INSTANTPLAYBACK_BUTTON,
    LIVEVIEW_SEQUENCING_BUTTON,
    LIVEVIEW_EXPAND_BUTTON,
    LIVEVIEW_CLOSE_BUTTON,
    MAX_LIVEVIEW_BUTTON
}LIVEVIEW_TOOLBAR_BUTTON_TYPE_e;

class LiveViewToolbarButton : public KeyBoard, public NavigationControl
{
    Q_OBJECT

private:
    WINDOW_ICON_TYPE_e m_windowIconType;
    LIVEVIEW_TOOLBAR_BUTTON_TYPE_e m_index;
    QString m_imageSource;
    STATE_TYPE_e m_currentState;
    IMAGE_TYPE_e m_currentImageType;
    bool m_isStateAvailable;
    QPixmap m_buttonImage;

    QTimer *m_clickEffectTimer;
    QPoint m_mouseMovePoint;

public:
    LiveViewToolbarButton(LIVEVIEW_TOOLBAR_BUTTON_TYPE_e index,
                          WINDOW_ICON_TYPE_e windowIconType,
                          QWidget *parent,
                          int indexInPage = 0,
                          bool isEnable = true);
    ~LiveViewToolbarButton();

    void changeButtonImage(IMAGE_TYPE_e type);
    void changeButtonState(STATE_TYPE_e state);
    void resetGeometry(qint16 offsetX);
    STATE_TYPE_e getButtonState();
    void selectControl();
    void deSelectControl();
    QPoint getMousePos() const;

    void forceActiveFocus();
    void setIsEnabled(bool isEnable);

    void takeEnterKeyAction();

    //virtual functions inherited from QWidget
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent * event);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    virtual void enterKeyPressed(QKeyEvent *event);

signals:
    void sigButtonClicked(LIVEVIEW_TOOLBAR_BUTTON_TYPE_e index, STATE_TYPE_e state);
    void sigUpdateCurrentElement(int index);
    void sigShowHideToolTip(int index, bool toShowTooltip);

public slots:
    void slotClickEffectTimeOut();
};


#endif // LIVEVIEWTOOLBARBUTTON_H
