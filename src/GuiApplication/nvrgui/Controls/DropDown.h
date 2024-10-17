/////////////////////////////////////////////////////////////////////////////
//   MMM     MMM       AAA       TTTTTTTTTT  RRRRRR    IIIIIIII  XX   XX
//   MMMM   MMMM      AA AA          TT      RR   RR      II      XX XX
//   MM MM MM MM     AA   AA         TT      RR    RR     II       XXX
//   MM  MM   MM    AAAAAAAAA        TT      RRRRRRR      II       XXX
//   MM       MM   AA       AA       TT      RR  RR       II      XX XX
//   MM       MM  AA         AA      TT      RR   RR   IIIIIIII  XX   XX
//
//   Company      : Matrix Telecom Pvt. Ltd., Baroda, India.
//   Project      : Satatya Products - TI
//   Owner        : Vandit Maniar
//   File         : DropDown.h
//   Description  :
/////////////////////////////////////////////////////////////////////////////

#ifndef DROPDOWN_H
#define DROPDOWN_H

#include <QPointer>
#include "../EnumFile.h"
#include "Controls/TextLabel.h"
#include "Controls/Bgtile.h"
#include "NavigationControl.h"
#include "Controls/InvisibleWidgetCntrl.h"
#include "Controls/DropDownList.h"
#include "Controls/ImageControls/Image.h"

typedef enum
{
    DROPDOWNBOX_SIZE_78,
    DROPDOWNBOX_SIZE_90,
    DROPDOWNBOX_SIZE_114,
    DROPDOWNBOX_SIZE_200,
    DROPDOWNBOX_SIZE_225,
    DROPDOWNBOX_SIZE_320,
    DROPDOWNBOX_SIZE_405,
    DROPDOWNBOX_SIZE_755,

    MAX_DROPDOWNBOX_SIZE
}DROPDOWNBOX_SIZE_e;

typedef enum
{
    DROPDOWNBOX_MAIN = 0,
    DROPDOWNBOX_DOWN_SCROLL

}DROPDOWNBOX_IMAGE_TYPE_e;

class DropDown : public BgTile,  public NavigationControl
{
    Q_OBJECT
public:
    explicit DropDown(quint32 startX,
                      quint32 startY,
                      quint32 width,
                      quint32 height,
                      quint16 controlIndex,
                      DROPDOWNBOX_SIZE_e butnSize,
                      QString labelStr,
                      QMap<quint8, QString>  listStr,
                      QWidget* parent = 0,
                      QString suffixStr = "",
                      bool isBoxStartInCentre = true,
                      quint16 leftMarginOfLabel = 0,
                      BGTILE_TYPE_e bgType = COMMON_LAYER,
                      bool isNavigationEnable = true,
                      quint8 maxElemetOnList = 8,
                      bool isBold = false,
                      bool isDropUpList = false,
                      quint16 leftMarginOfControl = 5,
                      quint32 leftMarginFromCenter = 0);

    ~DropDown();

    void createDefaultComponent(bool isBold);
    void changeImage (DROPDOWNBOX_IMAGE_TYPE_e index, IMAGE_TYPE_e imgType);

    void updateWithNextItem();
    void updateWithPrevItem();
    void setCurrValue(QString val);
    QString getCurrValue() const;
    void setIndexofCurrElement(quint8 index);
    quint8 getIndexofCurrElement() const;
    void changeTextAtIndex(quint8 index, QString newVal);
    void changeSuffixString(QString str);

    void appendInList(QString str);
    void setNewList(QMap<quint8, QString>  list, quint8 newSelectedIndex = 0);
    void setNewStringList(QStringList list, quint8 newSelectedIndex = 0);
    void selectControl();
    void deSelectControl();

    void loadDropList();
    void unloadDropList();

    void forceActiveFocus();
    void setIsEnabled(bool isEnable);

    void paintEvent (QPaintEvent *event);
    void mouseMoveEvent (QMouseEvent *event);
    void mousePressEvent (QMouseEvent *event);
    void mouseReleaseEvent (QMouseEvent *event);

    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void enterKeyPressed(QKeyEvent *event);

    
signals:
    void sigValueChanged(QString string, quint32 indexInPage);
    void sigUpdateCurrentElement(int index);
    void sigDropDownListDestroyed();
    
public slots:
    void slotDropListDestroyed();
    void slotValueChanged(quint8, QString);
    void slotUnloadDropList();

private:

    bool        m_isCentre;
    bool        m_isDropUpList;
    quint16     m_leftMarginOfLabel;
    quint16     m_leftMarginOfControl;
    quint32     m_leftMarginFromCenter;

    QString     m_label;
    QString     m_suffix;
    QMap<quint8, QString>  m_valueList;

    QString     m_imgPath;
    QPixmap     m_image;

    QString     m_imgDownScrollPath;
    QPixmap     m_imgDownScroll;

    QRect       m_imageRect;
    QRect       m_downScrollRect;

    DROPDOWNBOX_SIZE_e m_buttonSize;
    quint8      m_currListNo;
    quint8      m_maxListNo;

    TextLabel*             m_labelText;
    TextLabel*             m_suffixText;
    TextLabel*             m_listText;
    DropDownList*          m_dropDownList;
    InvisibleWidgetCntrl*  m_inVisibleWidget;

    IMAGE_TYPE_e    m_currentImageType;
    QPoint          m_lastClickPoint;
    
    quint8      m_maxElemetOnList;
};

#endif // DROPDOWN_H
