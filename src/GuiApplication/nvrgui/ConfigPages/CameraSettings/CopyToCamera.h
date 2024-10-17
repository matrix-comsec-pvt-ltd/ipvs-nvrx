#ifndef COPYTOCAMERA_H
#define COPYTOCAMERA_H

#include <QWidget>
#include "Elidedlabel.h"
#include "Controls/Rectangle.h"
#include "Controls/Closebuttton.h"
#include "Controls/ElementHeading.h"
#include "Controls/OptionSelectButton.h"
#include "Controls/TextLabel.h"
#include "Controls/ControlButton.h"
#include "Controls/CnfgButton.h"
#include "Controls/Heading.h"
#include "Controls/TextWithBackground.h"

#include "KeyBoard.h"

#define MAX_CAM_ON_PAGE 8
#define MAX_PAGE_NUMBER 4

typedef enum {
    CPY_CAM_CLOSE_BTN,
    CPY_CAM_ALL_CAMERA,
    CPY_CAM_CAMERA_LIST,
    CPY_CAM_PREV_BTN = (MAX_CAM_ON_PAGE + CPY_CAM_CAMERA_LIST),
    CPY_CAM_PAGE_NUM_BTN,
    CPY_CAM_NEXT_BTN = (CPY_CAM_PAGE_NUM_BTN + MAX_PAGE_NUMBER),
    CPY_CAM_OK_BTN,
    CPY_CAM_CANCEL_BTN,

    MAX_CPY_CAMERA_CONTROL
}CPY_CAMERA_CONTROL_e;

class CopyToCamera : public KeyBoard
{
    Q_OBJECT

public:
    CopyToCamera(QMap<quint8, QString> &cameralist,
                 CAMERA_BIT_MASK_t &cameraSelected,
                 QWidget *parent = 0,
                 QString headingLabel="",
                 quint8 indexInPage = 0,
                 bool footNoteNeeded = false);

    ~CopyToCamera();

    void showCameraList();
    void takeLeftKeyAction();
    void takeRightKeyAction();

    void showEvent (QShowEvent *event);
    void paintEvent (QPaintEvent *);

    void setAllCheckBoxState (OPTION_STATE_TYPE_e state);

    //keyboard support added
    void navigationKeyPressed(QKeyEvent *event);
    void backTab_KeyPressed(QKeyEvent *event);
    void tabKeyPressed(QKeyEvent *event);
    void escKeyPressed(QKeyEvent *event);
    void insertKeyPressed(QKeyEvent *event);

signals:
    void sigDeleteObject(quint8);

public slots:
    void slotButtonClick(int);
    void slotOptionButtonClicked(OPTION_STATE_TYPE_e,int);
    void slotUpdateCurrentElement(int index);
    void slotPageNumberButtonClick(QString);

private:
    bool                        nextPageSelected;
    bool                        isCopyToCameraEnable[MAX_CAMERAS];

    quint8                      currElement;
    quint8                      m_index;
    quint8                      totalCamera;
    quint8                      totalPages;
    quint8                      currentPageNum;
    quint32                     totalCameraSelected;

    CAMERA_BIT_MASK_t*          camSel;

    Rectangle*                  backGround;
    CloseButtton*               closeButton;
    ElementHeading*             elementHeading;
    ElementHeading*             elementHeadingElide;
    Heading*                    heading;
    ControlButton*              nextButton;
    ControlButton*              prevButton;
    TextLabel*                  cameraListLabel[MAX_CAM_ON_PAGE];
    CnfgButton*                 okButton;
    CnfgButton*                 cancelButton;
    ElidedLabel*                footnoteLabelElide;


    OptionSelectButton*         allCameraChecklist;
    OptionSelectButton*         cameraListCheckBox[MAX_CAM_ON_PAGE];
    QMap<quint8, QString>*      cameraNameList;

    NavigationControl*          m_elementlist[MAX_CPY_CAMERA_CONTROL];
    TextWithBackground*         m_PageNumberLabel[MAX_PAGE_NUMBER];
};

#endif // COPYTOCAMERA_H
