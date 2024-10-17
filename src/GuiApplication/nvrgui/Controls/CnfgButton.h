#ifndef CNFGBUTTON_H
#define CNFGBUTTON_H
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
#include "EnumFile.h"
#include "KeyBoard.h"
#include "Controls/TextLabel.h"
#include "NavigationControl.h"

#define CNFGBUTTON_FOLDER_PATH  IMAGE_PATH          "CnfgButtons/"

typedef enum
{
    CNFGBUTTON_MEDIAM,
    CNFGBUTTON_EXTRALARGE,
    MAX_CNFGBUTTON_SIZE
}CNFGBUTTON_SIZE_e;

const QString buttonSizeFolder[MAX_CNFGBUTTON_SIZE] =
{
    "Medium/",
    "ExtraLarge/"
};

class CnfgButton : public KeyBoard , public NavigationControl
{
    Q_OBJECT

private:
    QString label;
    quint8 fontSize;
    IMAGE_TYPE_e m_currentImageType;

    QRect imageRect;
    CNFGBUTTON_SIZE_e buttonSize;
    QString textColor;
    TextLabel *textLabel;
    QPixmap image;

    QPoint lastClickPoint;
    QTimer *clickeffectTimer;
    bool isDeletionStart;
public:
    CnfgButton(CNFGBUTTON_SIZE_e buttnSize,
               int centerX,
               int centerY,
               QString tLabel,
               QWidget *parent = 0,
               int indexInPage = 0,
               bool isEnabled = true);
    ~CnfgButton();

    void changeImage(IMAGE_TYPE_e type, bool isImageBoarderHiglights = false);
    IMAGE_TYPE_e getCurrentImageType();
    void changeText(QString str);
    QString getText();
    void changeColor(QString textcolor);
    void resetGeometry(quint32 startX, quint32 startY);
    void selectControl();
    void deSelectControl();

    void takeEnterKeyAction();

    void forceActiveFocus();
    void setIsEnabled(bool isEnable);

    //virtual functions inherited from QWidget
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
    void mouseMoveEvent (QMouseEvent *event);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    virtual void enterKeyPressed(QKeyEvent *event);

signals :
    void sigButtonClick(int index);
    void sigUpdateCurrentElement(int index);

public slots:
    void slotclickeffectTimerTimeout();
};
#endif // CNFGBUTTON_H
