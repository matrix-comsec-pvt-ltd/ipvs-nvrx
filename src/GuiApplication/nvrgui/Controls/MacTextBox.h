#ifndef MACTEXTBOX_H
#define MACTEXTBOX_H

#include <QWidget>
#include <QLineEdit>
#include <QPalette>
#include "Controls/Bgtile.h"
#include "Controls/TextLabel.h"
#include "Controls/Rectangle.h"
#include "VirtualKeypad.h"
#include "DataStructure.h"
#include "NavigationControl.h"
#include "Controls/LineEdit.h"
#include "Controls/InvisibleWidgetCntrl.h"

#define MAX_MAC_OCTAL               6

class MacTextBox : public BgTile, public NavigationControl
{
    Q_OBJECT
public:
    MacTextBox(quint32 startX,
              quint32 startY,
              quint32 width,
              quint32 height,
              quint16 controlIndex,
              QString labelstr,
              QWidget* parent = 0,
              BGTILE_TYPE_e bgType = COMMON_LAYER,
              bool isBoxInCentre = true,
              quint16 leftMarginVal = 0,
              bool isNavigationEnable = true,
              quint32 leftMarginFromCenter = 0);

    ~MacTextBox();
    void createDefaultComponent(void);
    void changeImage (IMAGE_TYPE_e imageType);
    void unloadVirtualKeypad();

    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    void mouseMoveEvent (QMouseEvent *event);
    void mousePressEvent (QMouseEvent *event);
    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void backspaceKeyPressed(QKeyEvent *event);
    virtual void enterKeyPressed(QKeyEvent *event);
    virtual void deleteKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);
    virtual void asciiCharKeyPressed(QKeyEvent *event);

    void hideEvent (QHideEvent *event);
    void showEvent (QShowEvent *event);

    void paintEvent (QPaintEvent *event);

    void selectControl();
    void deSelectControl();
    void setIsEnabled(bool isEnable);

    void forceActiveFocus();
    void mouseClickOnBox (bool isByRemote = false);

    void setCursorPos(bool isIncrement);
    void updateAllOctal(void);
    void doneKeyValidation(void);

    void setMacaddress(QString str);
    void getMacaddress(QString &str);
    QString getMacaddress();

private:
    QString m_label;
    bool m_isInCentre;
    bool entryByRemote;
    bool remoteCapsLock;
    bool editMode;
    bool isAllAddrEmpty;
    quint16 m_leftMargin;
    QPixmap m_image;
    quint8 m_currSelectedOctal;
    IMAGE_TYPE_e m_currentImageType;

    QRect m_imgRect;
    QPalette *m_palette;
    TextLabel *m_labelTextLabel;
    TextLabel *m_dotTextLabels[MAX_MAC_OCTAL - 1];
    LineEdit *m_octalLineEdit[MAX_MAC_OCTAL];

    VirtualKeypad *virtualKeypad;
    InvisibleWidgetCntrl *m_inVisibleWidget;

    quint8 currCurPos;

    QTimer *keyRepTimer;
    int prevRemoteKey;
    quint8 repeatKeyCnt;
    QString oldTextValue[MAX_MAC_OCTAL];
    QString currTextValue[MAX_MAC_OCTAL];
    quint32 m_leftMarginFromCenter;

signals:
    void sigUpdateCurrentElement(int);
    void sigLoadInfopage(quint32 index);

public slots:
    void slotkeyRepTimerTimeout();
    void slotLineEditFocusChange(quint8 index, bool isFocusIn, bool forceFocus);
    void slotTextBoxKeyPressed(KEY_TYPE_e keyType, QString str);
    void slotInvisibleCtrlMouseClick();
};

#endif // MACTEXTBOX_H
