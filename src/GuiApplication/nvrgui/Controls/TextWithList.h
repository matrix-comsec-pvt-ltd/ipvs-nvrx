#ifndef TEXTWITHLIST_H
#define TEXTWITHLIST_H

#include <QWidget>
#include <QPointer>
#include "../EnumFile.h"
#include <Controls/TextBox.h>
#include "Controls/TextLabel.h"
#include "Controls/Bgtile.h"
#include "NavigationControl.h"
#include "Controls/InvisibleWidgetCntrl.h"
#include "Controls/DropDownList.h"
#include "Controls/ImageControls/Image.h"

#define TEXTWITHLIST_IMAGE_PATH				IMAGE_PATH "TextWithList/"
#define TEXTWITHLIST_DOWN_ARROW_PATH       	IMAGE_PATH "Dropdown/Down_Arrow/"

typedef enum LIST_FILTER_TYPE_e
{
    LIST_FILTER_TYPE_INITIAL_CHAR_ONLY,
    LIST_FILTER_TYPE_ANY_CHAR

}LIST_FILTER_TYPE_e;

class TextWithList : public BgTile, public NavigationControl
{
    Q_OBJECT

public:
    explicit TextWithList(quint32 startX,
                          quint32 startY,
                          quint32 width,
                          quint32 height,
                          quint16 controlIndex,
                          QMap<quint8, QString> listStr,
                          QWidget *parent = 0,
                          TextboxParam *textBoxParam = 0,
                          BGTILE_TYPE_e bgType = COMMON_LAYER,
                          bool isNavigationEnable = true,
                          quint8 maxElemetOnList = 8,
                          TEXTBOX_SIZE_e butnSize = TEXTBOX_ULTRAMEDIAM,
                          QString placeHolderStr = "",
                          bool isDropUpList = false,
                          LIST_FILTER_TYPE_e filterType = LIST_FILTER_TYPE_INITIAL_CHAR_ONLY,
                          quint32 leftMarginFromCenter = 0);
    ~TextWithList();
    void createDefaultElement();
    void paintEvent (QPaintEvent *event);
    void changeImage(IMAGE_TYPE_e imgType);
    void setCurrValue(QString val, bool isClearText = false, bool forceDispText = false);
    void setIndexofCurrElement(quint8 index);
    void setNewList(QMap<quint8, QString>  list, quint8 newSelectedIndex = 0, bool clrDispText = false, bool forceDispText = false);
    void appendInList(QString str);
    void loadList(QMap<quint8, QString> maplist, quint8 listIndex);
    void unloadList();
    void selectControl();
    void deSelectControl();
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void hideEvent(QHideEvent *event);
    void showEvent(QShowEvent *event);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    void changeTextAtIndex(quint8 index, QString newVal);
    void unloadVirtualKeypad();
    void setIsEnabled(bool isEnable);
    void forceActiveFocus();
    QString getCurrValue(bool actualIndex = false) const;
    quint8 getIndexofCurrElement() const;

    virtual void unloadVirtualKeyboard();
    virtual void asciiCharKeyPressed(QKeyEvent *event);
    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);
    virtual void backspaceKeyPressed(QKeyEvent *event);
    virtual void enterKeyPressed(QKeyEvent *event);
    virtual void deleteKeyPressed(QKeyEvent *event);

signals:
    void sigUpdateCurrentElement(int index);
    void sigValueChanged(QString str, quint32 index);
    void sigValueListEmpty(quint8);

public slots:
    virtual void slotTextBoxKeyPressed(KEY_TYPE_e keyType, QString str);
    void slotLineEditFocusChange(quint8 index, bool isFocusIn, bool forceFocus);
    void slotTextValueAppended(QString str, int);
    void slotDropListDestroyed();
    void slotValueChanged(quint8 key, QString str);
    void slotUnloadDropList();

private:
    bool                        m_editMode;
    bool                        m_entryByRemote;
    bool                        m_mouseClickOnBox;
    bool                        m_isDropUpList;
    quint8                      m_currListNo;
    quint8                      m_maxElemetOnList;
    quint8                      m_maxListNo;
    quint16                     m_currCurPos;
    QMap<quint8, QString>       m_valueList;
    QStringList                 m_valueStringList;
    QString                     m_imagePath;
    QPixmap                     m_image;
    QRect                       m_imgRect;
    QPalette*                   m_palette;
    QString                     m_PlaceHolderText;
    TEXTBOX_SIZE_e              m_textBoxSize;
    IMAGE_TYPE_e                m_imgType;
    Image*                      m_downArrowImg;
    TextLabel*                  m_labelText;
    LineEdit*                   m_lineEdit;
    TextboxParam*               m_param;
    VirtualKeypad*              m_virtualKeypad;
    DropDownList*               m_dropDownList;
    InvisibleWidgetCntrl*       m_dropDownInVisibleWidget;
    quint16                     m_actualSelectedIndex;
    LIST_FILTER_TYPE_e          m_filterType;
    quint32                     m_leftMarginFromCenter;
};
#endif // TEXTWITHLIST_H
