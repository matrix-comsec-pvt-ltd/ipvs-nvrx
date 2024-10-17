#ifndef TOOLBARBUTTON_H
#define TOOLBARBUTTON_H

#include <QWidget>
#include "EnumFile.h"
#include "NavigationControl.h"
#include "KeyBoard.h"

class ToolbarButton : public KeyBoard, public NavigationControl
{
    Q_OBJECT

public:
    ToolbarButton(TOOLBAR_BUTTON_TYPE_e index,
                  QWidget * parent,
                  int indexInPage = 0,
                  int horizontalOffset = 0,
                  int verticalOffset = 0,
                  bool isEnable = true,
                  bool isHoverEffectNeeded = true);

    bool m_giveClickEffect;

    void changeButtonImage(IMAGE_TYPE_e type);
    void changeButtonState(STATE_TYPE_e state);
    STATE_TYPE_e getCurrentButtonState() const;
    TOOLBAR_BUTTON_TYPE_e getButtonIndex() const;
    IMAGE_TYPE_e getButtonImageType() const;

    void selectControl();
    void deSelectControl();

    void forceActiveFocus();
    void setIsEnabled(bool isEnable);

    void takeEnterKeyAction();

    void setOffset(int horizontalOffset, int verticalOffset);
    void updateGeometry();

    //virtual functions inherited from QWidget
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent * event);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    void mouseDoubleClickEvent(QMouseEvent *event);
    virtual void enterKeyPressed(QKeyEvent *event);
    ~ToolbarButton();
signals:
    void sigButtonClicked(TOOLBAR_BUTTON_TYPE_e index);
    void sigUpdateCurrentElement(int index);
    void sigShowHideToolTip(int index, bool toShowTooltip);
    void sigChangeMuteUnmute();
private:
    bool m_isStateAvailable;
    bool m_isMouseEffectNeeded;

    QPixmap m_buttonImage;
    QString m_imageSource;

    quint32 m_horizontalOffset, m_verticalOffset;

    STATE_TYPE_e m_currentState;
    IMAGE_TYPE_e m_currentImageType;
    TOOLBAR_BUTTON_TYPE_e m_index ,m_clickIndex;
    QTimer *clickEffectTimer;
public slots:
  void slotClickEffectTimerTimeout();
};

#endif // TOOLBARBUTTON_H
