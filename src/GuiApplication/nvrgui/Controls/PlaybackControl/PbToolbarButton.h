#ifndef PBTOOLBARBUTTON_H
#define PBTOOLBARBUTTON_H

#include "ApplController.h"
#include "EnumFile.h"
#include "KeyBoard.h"
#include "NavigationControl.h"
#define  PBTOOLBAR_IMAGE_PATH            IMAGE_PATH "PlayBackToolbar/"
typedef enum
{
    PB_BUTTON_STATE_1,
    PB_BUTTON_STATE_2,

    MAX_PB_BUTTON_STATE
}PB_BUTTON_STATE_TYPE_e;

typedef enum
{
    PB_TOOLBAR_1x1_2x2,
    PB_TOOLBAR_3x3,
    PB_TOOLBAR_4x4,
    PB_TOOLBAR_5x5,
    PB_TOOLBAR_6x6_8x8,

    MAX_PB_TOOLBAR_SIZE
}PB_TOOLBAR_SIZE_e;

typedef enum
{
    PB_TOOLBAR_PLAY,
    PB_TOOLBAR_STOP,
    PB_TOOLBAR_REVERSE_PLAY,
    PB_TOOLBAR_SLOW,
    PB_TOOLBAR_FAST,
    PB_TOOLBAR_PREVIOUS,
    PB_TOOLBAR_NEXT,
    PB_TOOLBAR_MUTE,
    PB_TOOLBAR_PAUSE,
    PB_TOOLBAR_UNMUTE,

    MAX_PB_TOOLBAR_BTN
}PBTOOLBAR_BTN_IMAGE_e;

const QString pbButtonSizeFolder[MAX_PB_TOOLBAR_SIZE] =
{
    "1x1_2x2/",
    "3x3/",
    "4x4/",
    "5x5/",
    "6x6_8x8/"
};

class PbToolbarButton : public KeyBoard, public NavigationControl
{
    Q_OBJECT
private:
    int m_startX, m_startY;
    PB_TOOLBAR_SIZE_e m_toolBarSize;
    PBTOOLBAR_BTN_IMAGE_e m_toolBarBtn;
    IMAGE_TYPE_e m_currentImageType;
    QPixmap m_image;
    QRect m_imageRect;
    QTimer *clickeffctTimer;
    QPoint lastClickPoint;

public:
    PbToolbarButton(quint16 xParam,
                    quint16 yParam,
                    PB_TOOLBAR_SIZE_e toolbarSize,
                    QWidget *parent = 0,
                    int indexInPage = 0,
                    bool isEnabled = true,
                    bool catchKey = true,
                    int buttonTypeIndex = 0);

    ~PbToolbarButton();

    void setGeometryForElements();
    void changeImage(IMAGE_TYPE_e type);

    void selectControl();
    void deSelectControl();

    void forceActiveFocus();
    void takeEnterKeyAction();
    void setIsEnabled(bool isEnable);

    void changeToolbarBtn(PBTOOLBAR_BTN_IMAGE_e img);
    PBTOOLBAR_BTN_IMAGE_e getToolbarBtn();

    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    virtual void enterKeyPressed(QKeyEvent *event);
    bool eventFilter (QObject *, QEvent *);    
    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);

signals :
    void sigButtonClick(int indexInPage);
    void sigUpdateCurrentElement(int index);
    void sigImageMouseHover(quint8,bool);

public slots:
    void slotclickeffctTimerTimeout();
};

#endif // PBTOOLBARBUTTON_H
