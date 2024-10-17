#ifndef MESSAGEBOX_H
#define MESSAGEBOX_H

#include <QWidget>
#include <QTextEdit>
#include <QPalette>
#include "Controls/Bgtile.h"
#include "Controls/TextLabel.h"
#include "Controls/Rectangle.h"
#include "VirtualKeypad.h"
#include "DataStructure.h"
#include "NavigationControl.h"
#include "Controls/TextEdit.h"
#include "Controls/InvisibleWidgetCntrl.h"

class MessageBox :  public BgTile, public NavigationControl
{
    Q_OBJECT

    bool        isFocusIn;
    bool        editMode;
    bool        entryByRemote;
    bool        remoteCapsLock;
    bool        isCentre;
    bool        m_isCharCountExternallyControl;
    bool        m_isSpecialFunDone;
    bool        m_isSpecialCharCountNeeded;

    QPalette    *palette;
    QTimer      *keyRepTimer;

    QPixmap     image;
    QRect       imgRect;
    QRegExp     validation;

    quint8      repeatKeyCnt;
    quint16     leftMargin;
    quint16     indexOfControl;
    quint16     maxChar;
    QString     labelStr;
    QString     imagePath;
    QString     oldTextValue;
    QString     textStr;

    IMAGE_TYPE_e imgType;

    TextLabel   *labelText;
    TextEdit    *textEdit;

    VirtualKeypad *virtualKeypad;
    InvisibleWidgetCntrl *m_inVisibleWidget;

    int prevRemoteKey;
    int currCurPos;

    quint32 currentCharLenght;
    quint32 m_leftMarginFromCenter;

public:
    explicit MessageBox(quint32 startX,
                        quint32 startY,
                        quint32 width,
                        quint16 controlIndex,
                        QWidget *parent = 0,
                        QString label = "",
                        BGTILE_TYPE_e bgType = COMMON_LAYER,
                        bool iscentre = true,
                        quint16 leftMgn = 0,
                        quint16 maxChr = 100,
                        QRegExp valdtion = QRegExp(QString("[a-zA-Z0-9]")),
                        bool isNavigationEnable = true,
                        bool isCharCountExternallyControl = false,
                        bool isSpecialCharCountNeeded = true,
                        quint32 leftMarginFromCenter = 0);

    ~MessageBox();

    void createDefaultElement();
    void unloadVirtualKeypad();

    QString getInputText();
    void setInputText(QString);
    void setCurrentCharLenght(quint8 maxCharCount);

    void changeImage(IMAGE_TYPE_e imgType);

    void setIsEnabled(bool isEnable);
    void selectControl();
    void deSelectControl();
    void forceActiveFocus();

    void mouseClickOnBox(bool isByRemote = false);
    void mousePressEvent (QMouseEvent *event);
    void mouseMoveEvent (QMouseEvent *event);
    void hideEvent (QHideEvent *event);
    void showEvent (QShowEvent *event);
    void paintEvent (QPaintEvent *event);

    void focusInEvent(QFocusEvent *);
    void unloadVirtualKeyboard();
    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void backspaceKeyPressed(QKeyEvent *event);
    virtual void enterKeyPressed(QKeyEvent *event);
    virtual void deleteKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);
    virtual void asciiCharKeyPressed(QKeyEvent *event);

    //shortcut key functions
    virtual void ctrl_A_KeyPressed(QKeyEvent *event);
    virtual void ctrl_C_KeyPressed(QKeyEvent *event);
    virtual void ctrl_V_KeyPressed(QKeyEvent *event);
    virtual void ctrl_X_KeyPressed(QKeyEvent *event);
    virtual void ctrl_Y_KeyPressed(QKeyEvent *event);
    virtual void ctrl_Z_KeyPressed(QKeyEvent *event);

    void focusOutEvent(QFocusEvent *);
    void take0to9KeyAction(int key);
    quint16 countStrLenWithSpChr(QString str);
    quint16 countActualCharLenToTruncate(QString str, quint16 count);

signals:
    void sigUpdateCurrentElement(int index);
    void sigTextValueAppended(QString str, int index);

public slots:
    void slotkeyRepTimerTimeout();
    void slotTextBoxKeyPressed(KEY_TYPE_e keyType, QString str);
    void slotTextEditFocusChange(bool tIsFocusIn, bool forceFocus);
    void slotInvisibleCtrlMouseClick();
};

#endif // MESSAGEBOX_H
