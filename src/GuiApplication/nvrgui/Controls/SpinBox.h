#ifndef SPINBOX_H
#define SPINBOX_H
/////////////////////////////////////////////////////////////////////////////
//   MMM     MMM       AAA       TTTTTTTTTT  RRRRRR    IIIIIIII  XX   XX
//   MMMM   MMMM      AA AA          TT      RR   RR      II      XX XX
//   MM MM MM MM     AA   AA         TT      RR    RR     II       XXX
//   MM  MM   MM    AAAAAAAAA        TT      RRRRRRR      II       XXX
//   MM       MM   AA       AA       TT      RR  RR       II      XX XX
//   MM       MM  AA         AA      TT      RR   RR   IIIIIIII  XX   XX
//
//   Company      : Matrix Telecom Pvt. Ltd., Baroda, India.
//   Project      : DVR (Digital Video Recorder - TI)
//   Owner        : Tushar Rabadiya
//   File         : PageOpenButton.cpp
//   Description  :
/////////////////////////////////////////////////////////////////////////////
#include "ApplController.h"
#include "../EnumFile.h"
#include "Controls/TextLabel.h"
#include "Controls/Bgtile.h"
#include "NavigationControl.h"

typedef enum
{
    SPINBOX_SIZE_78,
    SPINBOX_SIZE_90,

    MAX_SPINBOX_SIZE
}SPINBOX_SIZE_e;

typedef enum
{
    SPINBOX_MAIN = 0,
    SPINBOX_UPPER_SCROLL,
    SPINBOX_DOWN_SCROLL
}SPINBOX_IMAGE_TYPE_e;

class SpinBox: public BgTile,  public NavigationControl
{
    Q_OBJECT

public:
    SpinBox(quint32 startX,
            quint32 startY,
            quint32 width,
            quint32 height,
            quint16 controlIndex,
            SPINBOX_SIZE_e butnSize,
            QString labelStr,
            QStringList listStr,
            QWidget* parent = 0,
            QString suffixStr = "",
            bool isBoxStartInCentre = true,
            quint16 leftMarginOfLabel = 0,
            BGTILE_TYPE_e bgType = COMMON_LAYER,
            bool isNavigationEnable = true);

    ~SpinBox();

private:
    bool isCentre;
    quint16 leftMargin;

    QString label;
    QString suffix;
    QStringList valueList;

    QString imgPath;
    QPixmap image;

    QString imgUpperScrollPath;
    QPixmap imgUpperScroll;
    QString imgDownScrollPath;
    QPixmap imgDownScroll;

    QRect imageRect;
    QRect upperScrollRect;
    QRect downScrollRect;

    SPINBOX_SIZE_e buttonSize;
    quint8 currListNo;
    quint8 maxListNo;

    TextLabel *labelText;
    TextLabel *suffixText;
    TextLabel *listText;

    IMAGE_TYPE_e m_currentImageType;
    bool isMouseClick;
    QPoint lastClickPoint;

public:
    void createDefaultComponent();
    void changeImage (SPINBOX_IMAGE_TYPE_e index, IMAGE_TYPE_e imgType);

    void updateWithNextItem();
    void updateWithPrevItem();
    void setCurrValue(QString val);
    QString getCurrValue();
    void setIndexofCurrElement(quint8 index);
    quint8 getIndexofCurrElement(void);
    void changeTextAtIndex(quint8 index, QString newVal);
    void changeSuffixString(QString str);

    void appendInList(QString str);
    void setNewList(QStringList list, quint8 newSelectedIndex = 0);
    void selectControl();
    void deSelectControl();

    void forceActiveFocus();
    void setIsEnabled(bool isEnable);

    void paintEvent (QPaintEvent *event);
    void mouseMoveEvent (QMouseEvent *event);
    void mousePressEvent (QMouseEvent *event);
    void mouseReleaseEvent (QMouseEvent *event);
    void wheelEvent (QWheelEvent *event);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    virtual void navigationKeyPressed(QKeyEvent *event);

signals:
    void sigValueChanged(QString string, quint32 indexInPage);
    void sigUpdateCurrentElement(int index);
};

#endif // SPINBOX_H
