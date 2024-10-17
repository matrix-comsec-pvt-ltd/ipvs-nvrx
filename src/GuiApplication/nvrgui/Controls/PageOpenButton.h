#ifndef PAGEOPENBUTTON_H
#define PAGEOPENBUTTON_H
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
#include <QWidget>
#include <QTimer>
#include "ApplController.h"
#include "../EnumFile.h"
#include "Controls/TextLabel.h"
#include "Controls/Bgtile.h"
#include "NavigationControl.h"

typedef enum
{
    PAGEOPENBUTTON_SMALL,
    PAGEOPENBUTTON_MEDIAM_BACK,
    PAGEOPENBUTTON_MEDIAM_NEXT,
    PAGEOPENBUTTON_LARGE,
    PAGEOPENBUTTON_EXTRALARGE,
    PAGEOPENBUTTON_ULTRALARGE,
    PAGEOPENBUTTON_LARGE_BACK,
    PAGEOPENBUTTON_ULTRALARGE_IMGOVERLAPED,

    MAX_PAGEOPENBUTTON_SIZE
}PAGEOPENBUTTON_SIZE_e;


class PageOpenButton : public BgTile,public NavigationControl
{
    Q_OBJECT
public:
    PageOpenButton(quint32 startX,
                   quint32 startY,
                   quint32 width,
                   quint32 height,
                   quint16 controlIndex,
                   PAGEOPENBUTTON_SIZE_e buttnSize,
                   QString btnNameStr,
                   QWidget *parent = 0,
                   QString labelStr = "",
                   QString suffixStr = "",
                   bool isBoxStartInCentre = true,
                   quint16 leftMarginOfLabel = 0,
                   BGTILE_TYPE_e bgType = COMMON_LAYER,
                   bool isNavigationEnable = true,
                   TEXTLABEL_ALIGNMENT_e textAlignment = ALIGN_START_X_CENTRE_Y,
                   quint32 leftMarginFromCenter = 0);
    ~PageOpenButton();

private:
    QString btnName;
    QString label;
    QString suffix;
    TEXTLABEL_ALIGNMENT_e m_textAlign;
    bool isInCentre;

    QString imgPath;
    quint16 leftMargin;
    QRect imageRect;
    PAGEOPENBUTTON_SIZE_e buttonSize;
    QString textColor;
    TextLabel *btnText;
    TextLabel *labelText;
    TextLabel *suffixText;

    QPixmap image;

    QPoint lastClickPoint;
    qint8 m_currentImageType;
    QTimer *clickeffctTimer;
    quint32 m_leftMarginFromCenter;

public:
    void createDefaultComponent();
    void changeImage(IMAGE_TYPE_e type);
    void resetGeometry (quint32 startX,
                        quint32 startY);
    void selectControl();
    void deSelectControl();

    void forceActiveFocus();
    void setIsEnabled(bool isEnable);

    void takeEnterKeyAction();

    //virtual functions inherited from QWidget
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    virtual void enterKeyPressed(QKeyEvent *event);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
    void mouseMoveEvent (QMouseEvent *event);

signals :
    void sigButtonClick(int indexInPage);
    void sigUpdateCurrentElement(int index);

public slots:
    void slotclickeffctTimerTimeout();
};
#endif // PAGEOPENBUTTON_H
