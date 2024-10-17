#ifndef CAMSEARCHFAILREPORT_H
#define CAMSEARCHFAILREPORT_H

#include <QWidget>
#include "Controls/Rectangle.h"
#include "Controls/Heading.h"
#include "Controls/Closebuttton.h"
#include "Controls/ControlButton.h"
#include "Controls/TableCell.h"
#include "Controls/TextLabel.h"
#include "KeyBoard.h"

#define MAX_REPORT_ON_PAGE      8
#define MAX_FAIL_RECORD_DATA   (MAX_REPORT_ON_PAGE+1)

typedef enum
{
    CAM_SRCH_CLS_BTN,
    CAM_SRCH_PREV_BTN,
    CAM_SRCH_NEXT_BTN,
    MAX_CAM_SRCH_CTRL
}CAM_SRCH_CTRL_e;

class CamSearchFailReport : public KeyBoard
{
    Q_OBJECT
public:
    explicit CamSearchFailReport(QStringList ipAddrList,
                                 QStringList failStatus,
                                 QWidget *parent = 0);
    
    ~CamSearchFailReport();

    void showEvent (QShowEvent *event);
    void paintEvent (QPaintEvent *);

    //keyboard support added
    virtual void navigationKeyPressed(QKeyEvent *event);
    virtual void escKeyPressed(QKeyEvent *event);
    virtual void insertKeyPressed(QKeyEvent *event);
    virtual void tabKeyPressed(QKeyEvent *event);
    virtual void backTab_KeyPressed(QKeyEvent *event);

signals:
    void sigObjectDelete();
    
public slots:
    void slotButtonClick(int indexInPage);
    void slotUpdateCurrentElement(int indexInPage);

private:

    Rectangle* backGround;
    CloseButtton* closeButton;
    Heading* heading;

    QStringList ipAddressList;
    QStringList failureStatusList;

    NavigationControl*  m_elementlist[MAX_CAM_SRCH_CTRL];
    quint8              currElement;

    TableCell*          srNumber[MAX_FAIL_RECORD_DATA];
    TableCell*          ipAddress[MAX_FAIL_RECORD_DATA];
    TableCell*          failureStatus[MAX_FAIL_RECORD_DATA];

    TextLabel*          srNumberStr[MAX_FAIL_RECORD_DATA];
    TextLabel*          ipAddressStr[MAX_FAIL_RECORD_DATA];
    TextLabel*          failureStatusStr[MAX_FAIL_RECORD_DATA];

    ControlButton* nextButton;
    ControlButton* prevButton;

    quint8 currentPageNum;
    quint8 totalReport;
    quint8 maximumPages;

    void createDefaultComponents();
    void showReports();
    void takeLeftKeyAction();
    void takeRightKeyAction();

};

#endif // CAMSEARCHFAILREPORT_H
