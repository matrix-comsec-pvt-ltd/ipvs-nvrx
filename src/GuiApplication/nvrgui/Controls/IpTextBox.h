#ifndef IPTEXTBOX_H
#define IPTEXTBOX_H

#include <arpa/inet.h>

#include <QWidget>
#include <QLineEdit>
#include <QPalette>
#include <QHostAddress>

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
    IP_FIELD_TYPE_IPV6_ADDR,
    IP_FIELD_TYPE_IPV6_GATEWAY,
    IP_FIELD_TYPE_DNSV6,
    IP_FIELD_TYPE_STATIC_ROUTE_NW_ADDR,
    MAX_IP_TEXTBOX_FIELD_TYPE
}IP_FIELD_TYPE_e;

typedef enum
{
    IP_ADDR_TYPE_IPV4_ONLY,
    IP_ADDR_TYPE_IPV6_ONLY,
    IP_ADDR_TYPE_IPV4_AND_IPV6,
    MAX_IP_ADDR_TYPE
}IP_ADDR_TYPE_e;

typedef enum
{
    IP_TEXTBOX_SMALL,
    IP_TEXTBOX_MEDIAM,
    IP_TEXTBOX_ULTRAMEDIAM,
    IP_TEXTBOX_LARGE,
    IP_TEXTBOX_ULTRALARGE,
    IP_TEXTBOX_EXTRASMALL,
    IP_TEXTBOX_EXTRALARGE,
    IP_TEXTBOX_TABLE_TYPE_SMALL,
    MAX_IP_TEXTBOX_SIZE
}IP_TEXTBOX_SIZE_e;

class IpTextBox : public BgTile, public NavigationControl
{
    Q_OBJECT
public:
    IpTextBox(quint32 startX,
              quint32 startY,
              quint32 width,
              quint32 height,
              quint16 controlIndex,
              QString labelstr,
              IP_ADDR_TYPE_e ipAddressType,
              QWidget* parent = 0,
              BGTILE_TYPE_e bgType = COMMON_LAYER,
              bool isBoxInCentre = true,
              quint16 leftMarginVal = 0,
              bool isNavigationEnable = true,
              IP_FIELD_TYPE_e fieldType = IP_FIELD_TYPE_IPV6_ADDR,
              IP_TEXTBOX_SIZE_e size = IP_TEXTBOX_LARGE,
              quint32 leftMarginFromCenter = 0);

    ~IpTextBox();
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
    void doneKeyValidation(void);

    void setIpaddress(QString str);
    void getIpaddress(QString &str);
    QString getIpaddress();

    bool operator > (const IpTextBox &other) const;
    bool operator < (const IpTextBox &other) const;

private:
    QString                 m_label;
    bool                    m_isInCentre;
    bool                    entryByRemote;
    bool                    editMode;
    quint16                 m_leftMargin;
    QPixmap                 m_image;
    IMAGE_TYPE_e            m_currentImageType;
    QRect                   m_imgRect;

    QPalette                *m_palette;
    TextLabel               *m_labelTextLabel;
    LineEdit                *m_lineEdit;

    VirtualKeypad           *virtualKeypad;
    InvisibleWidgetCntrl    *m_inVisibleWidget;
    InvisibleWidgetCntrl    *m_physicalKeyboardInVisibleWidget;

    quint8                  currCurPos;
    QString                 oldTextValue;
    QString                 currTextValue;
    IP_FIELD_TYPE_e         m_fieldType;
    IP_ADDR_TYPE_e          m_ipAddressType;
    QRegExp                 alphaNumRegExp;
	IP_TEXTBOX_SIZE_e       textBoxSize;
    quint32                 m_leftMarginFromCenter = 0;
signals:
    void sigUpdateCurrentElement(int index);
    void sigLoadInfopage(quint32 index);
    void sigEntryDone(quint32 index);

public slots:
    void slotTextBoxKeyPressed(KEY_TYPE_e keyType, QString str);
    void slotInvisibleCtrlMouseClick();
};

#endif // IPTEXTBOX_H
