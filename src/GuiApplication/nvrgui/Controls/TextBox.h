#ifndef TEXTBOX_H
#define TEXTBOX_H

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

typedef enum
{
    INFO_MSG_STRAT_CHAR,
    INFO_MSG_END_CHAR,
    INFO_MSG_ERROR,
    INFO_MSG_MAX_CHAR
}INFO_MSG_TYPE_e;

typedef enum
{
    TEXTBOX_SMALL,
    TEXTBOX_MEDIAM,
    TEXTBOX_ULTRAMEDIAM,
    TEXTBOX_LARGE,
    TEXTBOX_ULTRALARGE,
    TEXTBOX_EXTRASMALL,
    TEXTBOX_EXTRALARGE,
	TEXTBOX_TABLE_TYPE_SMALL,
    MAX_TEXTBOX_SIZE
}TEXTBOX_SIZE_e;

class TextBox : public BgTile, public NavigationControl
{
    Q_OBJECT
public:
    TextBox(quint32 startX,
            quint32 startY,
            quint32 width,
            quint32 height,
            quint16 controlIndex,
            TEXTBOX_SIZE_e size,
            QWidget* parent = 0,
            TextboxParam *textBoxParam = 0,
            BGTILE_TYPE_e bgType = COMMON_LAYER,
            bool isNavigationEnable = true,
            bool isCharCountExternallyControl = false,
            bool isSpecialCharCountNeeded = false,
            quint32 leftMarginFromCenter = 0);

    ~TextBox();

    void createDefaultElement();
    void paintEvent (QPaintEvent *event);
    void changeImage(IMAGE_TYPE_e imgType);
    bool doneKeyValidation();
    void unloadVirtualKeypad();
    void createPhysicalKeyboardWidget(void);
    void deletePhysicalKeyboardWidget(void);

    void setCurrentCharLenght(quint8 maxCharCount);
    void setIsEnabled(bool isEnable);

    void selectControl();
    void deSelectControl();
    void forceActiveFocus();
    void takeEnterKeyAction();

    void mouseClickOnBox(bool isByRemote = false);
    void mousePressEvent (QMouseEvent *event);
    void mouseMoveEvent (QMouseEvent *event);
    void hideEvent (QHideEvent *event);
    void showEvent (QShowEvent *event);

    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);

    void take0to9KeyAction(int key);
    QString getInputText() const;
    void getInputText(QString &str);
    virtual void setInputText(QString str);
    void raiseVirtualKeypad();
    quint16 countStrLenWithSpChr(QString str);
    quint16 countActualCharLenToTruncate(QString str, quint16 count);

    virtual void unloadVirtualKeyboard();
    virtual void asciiCharKeyPressed(QKeyEvent *event);
    virtual void navigationKeyPressed(QKeyEvent *event);

    //misc key functions
    virtual void escKeyPressed(QKeyEvent *event);
    virtual void backspaceKeyPressed(QKeyEvent *event);
    virtual void enterKeyPressed(QKeyEvent *event);
    virtual void deleteKeyPressed(QKeyEvent *event);

    //shortCut key functions
    virtual void ctrl_A_KeyPressed(QKeyEvent *event);
    virtual void ctrl_C_KeyPressed(QKeyEvent *event);
    virtual void ctrl_V_KeyPressed(QKeyEvent *event);
    virtual void ctrl_X_KeyPressed(QKeyEvent *event);
    virtual void ctrl_Y_KeyPressed(QKeyEvent *event);
    virtual void ctrl_Z_KeyPressed(QKeyEvent *event);

protected:
    quint16         indexOfControl;
    int             prevRemoteKey;
    bool            editMode;
    quint8          repeatKeyCnt;
    TEXTBOX_SIZE_e  textBoxSize;
    IMAGE_TYPE_e    imgType;
    quint16         currCurPos;
    QString         oldTextValue;

    bool            entryByRemote;
    bool            remoteCapsLock;
    QString         imagePath;
    QPixmap         image;
    QRect           imgRect;

    VirtualKeypad*  virtualKeypad;
    QTimer*         keyRepTimer;
    TextLabel*      labelText;
    TextLabel*      suffixText;
    QPalette*       palette;
    LineEdit*       lineEdit;

    TextboxParam*           param;
    InvisibleWidgetCntrl    *m_inVisibleWidget;
    InvisibleWidgetCntrl    *m_physicalKeyboardInVisibleWidget;

    bool            reloadListNeedded;
    quint32         currentCharLenght;
    bool            m_isCharCountExternallyControl;
    bool            m_isSpecialFunDone;
    bool            m_isSpecialCharCountNeeded;
    quint32         m_leftMarginFromCenter;

signals:
    void sigUpdateCurrentElement(int index);
    void sigLoadInfopage(int index, INFO_MSG_TYPE_e msgType);
    void sigTextValueAppended(QString str, int index);

public slots:
    void slotkeyRepTimerTimeout();
    virtual void slotTextBoxKeyPressed(KEY_TYPE_e keyType, QString str);
    void slotInvisibleCtrlMouseClick();
};

#endif // TEXTBOX_H
