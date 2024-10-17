#ifndef IPV4TEXTBOX_H
#define IPV4TEXTBOX_H

#include <QWidget>
#include <QLineEdit>
#include <QPalette>
#include <bitset>
#include <QHostAddress>
#include "Controls/Bgtile.h"
#include "Controls/TextLabel.h"
#include "Controls/Rectangle.h"
#include "VirtualKeypad.h"
#include "DataStructure.h"
#include "NavigationControl.h"
#include "Controls/LineEdit.h"
#include "Controls/InvisibleWidgetCntrl.h"

#define MAX_OCTAL   4

class Ipv4TextBox : public BgTile, public NavigationControl
{
    Q_OBJECT
public:
    Ipv4TextBox(quint32 startX,
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
                bool isSubnetMask = false,
                quint32 leftMarginFromCenter = 0);
    ~Ipv4TextBox();

    void createDefaultComponent(void);
    void changeImage(IMAGE_TYPE_e imageType);
    void unloadVirtualKeypad();

    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void createPhysicalKeyboardWidget();
    void deletePhysicalKeyboardWidget();

    virtual void unloadVirtualKeyboard();
    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void backspaceKeyPressed(QKeyEvent *event);
    virtual void enterKeyPressed(QKeyEvent *event);
    virtual void deleteKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);
    virtual void asciiCharKeyPressed(QKeyEvent *event);

    void hideEvent(QHideEvent *event);
    void showEvent(QShowEvent *event);
    void paintEvent(QPaintEvent *event);

    void selectControl();
    void deSelectControl();
    void setIsEnabled(bool isEnable);

    void forceActiveFocus();
    void mouseClickOnBox(bool isByRemote = false);

    void setCursorPos(bool isIncrement);
    void updateAllOctal(void);
    void doneKeyValidation(void);

    void setIpaddress(QString str);
    void getIpaddress(QString &str);
    QString getIpaddress();
    QString getCurrentIpAddress();

    QString getSubnetOfIpaddress();
    QString getlastOctalOfIpaddress();
    QString getSubnetToPrefixLength();

private:
    QString         m_label;
    bool            m_isSubnetMask;
    bool            m_isInCentre;
    bool            entryByRemote;
    bool            editMode;
    bool            isAllAddrEmpty;
    quint16         m_leftMargin;
    quint32         m_leftMarginFromCenter;
    QPixmap         m_image;
    quint8          m_currSelectedOctal;
    IMAGE_TYPE_e    m_currentImageType;

    QRect       m_imgRect;
    QPalette    *m_palette;
    TextLabel   *m_labelTextLabel;
    TextLabel   *m_dotTextLabels[MAX_OCTAL-1];
    LineEdit    *m_octalLineEdit[MAX_OCTAL];

    VirtualKeypad           *virtualKeypad;
    InvisibleWidgetCntrl    *m_inVisibleWidget;
    InvisibleWidgetCntrl    *m_physicalKeyboardInVisibleWidget;

    quint8 currCurPos;
    QString oldTextValue[MAX_OCTAL];
    QString currTextValue[MAX_OCTAL];

signals:
    void sigUpdateCurrentElement(int index);
    void sigLoadInfopage(quint32 index);
    void sigEntryDone(quint32 index);

public slots:
    void slotTextBoxKeyPressed(KEY_TYPE_e keyType, QString str);
    void slotLineEditFocusChange(quint8 index, bool isFocusIn, bool forceFocus);
    void slotInvisibleCtrlMouseClick();
};

#endif // IPV4TEXTBOX_H
