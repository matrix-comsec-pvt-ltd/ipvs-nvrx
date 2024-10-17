#ifndef RECORDRETENTION_H
#define RECORDRETENTION_H

#include <QWidget>

#include "Controls/TextLabel.h"
#include "Controls/Rectangle.h"
#include "Controls/Heading.h"
#include "Controls/Closebuttton.h"
#include "Controls/ElementHeading.h"
#include "Controls/TextBox.h"
#include "Controls/ControlButton.h"
#include "Controls/CnfgButton.h"
#include "Controls/InfoPage.h"
#include "DataStructure.h"
#include "Controls/TextWithBackground.h"

#define MAX_CAM_ON_PAGE 8
#define MAX_PAGE_NUMBER 4

typedef enum{

    REC_RETN_CLS_BTN,
    REC_RETN_TXTBX,
    REC_PREV_BTN = (MAX_CAM_ON_PAGE + REC_RETN_TXTBX),
    REC_PAGE_NUMBER_BTN,
    REC_NEXT_BTN = (REC_PAGE_NUMBER_BTN + MAX_PAGE_NUMBER),
    REC_OK_BTN,
    REC_CANL_BTN,

    MAX_REC_RETN_CTRL
}REC_RETN_CTRL_e;

class RecordRetention : public KeyBoard
{
    Q_OBJECT
public:
    explicit RecordRetention(QStringList &camlist,
                             QStringList &retnDays,
                             QWidget *parent = 0,
                             QString headingLabel="",
                             quint8 indexInPage = 0);

    ~RecordRetention();

    void showRecords();
    bool saveRecords();
    void takeLeftKeyAction();
    void takeRightKeyAction();

    void paintEvent (QPaintEvent *event);
    void showEvent (QShowEvent *event);
    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);
    virtual void tabKeyPressed(QKeyEvent *event);
    virtual void backTab_KeyPressed(QKeyEvent *event);
    virtual void insertKeyPressed(QKeyEvent *event);

signals:
    void sigDeleteObject(quint8);

public slots:
    void slotButtonClick(int);
    void slotUpdateCurrentElement(int);
    void slotInfoPageBtnclick(int);
    void slotLoadInfoPage(int,INFO_MSG_TYPE_e);
    void slotPageNumberButtonClick(QString);

private:

    bool            nextPageSelected;
    quint8          m_index;
    quint8          totalCamera;
    quint8          currentPageNum;
    quint8          totalPages;

    InfoPage*       infoPage;
    QStringList*    recList;
    QStringList*    camList;

    Rectangle*      backGround;
    CloseButtton*   closeButton;
    ElementHeading* elementHeading;
    ElementHeading* elementHeading1;
    Heading*        heading;

    BgTile*         cameraTile[MAX_CAM_ON_PAGE];
    TextLabel*      cameraNames[MAX_CAM_ON_PAGE];

    TextboxParam*   retainDaysTextboxParam[MAX_CAM_ON_PAGE];
    TextBox*        retainDaysTextbox[MAX_CAM_ON_PAGE];

    ControlButton*  nextButton;
    ControlButton*  prevButton;
    CnfgButton*     okButton;
    CnfgButton*     cancelButton;

    NavigationControl* m_elementlist[MAX_REC_RETN_CTRL];
    quint8          currElement;

    TextWithBackground*     m_PageNumberLabel[MAX_PAGE_NUMBER];
};

#endif // RECORDRETENTION_H
