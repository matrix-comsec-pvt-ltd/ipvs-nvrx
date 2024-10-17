#ifndef MXQUICKBCKUPREPORTTABLE_H
#define MXQUICKBCKUPREPORTTABLE_H

#include <QWidget>
#include <QKeyEvent>
#include "Controls/Rectangle.h"
#include "Controls/Heading.h"
#include "Controls/Closebuttton.h"
#include "Controls/ControlButton.h"
#include "Controls/TableCell.h"
#include "Controls/TextLabel.h"
#include "Controls/BackGround.h"
#include "NavigationControl.h"
#include "MxTable.h"

#define MAX_QUICK_BKUP_RPT_DATA_COL   8
#define MAX_QUICK_BKUP_RPT_DATA_ROW   10

// List of control
typedef enum
{
    QUICK_BACK_CLOSE_BTN,
    QUICK_BACK_TABLE_PAGE,

    MAX_QUICK_BACK_CTRL
}QUICK_BACK_CTRL_e;

class MxQuickBckupReportTable : public BackGround
{
    Q_OBJECT
private:
    int               m_currElement;
    //BgTile*           m_insideQuickBkpTile;

    TextLabel*        m_totalCameraFoundTextLableStr;
    TextLabel*        m_sucess_TextLableStr;
    TextLabel*        m_failedTextLableStr;
    TextLabel*        m_totalCameraLableStr;
    TextLabel*        m_successLableStr;
    TextLabel*        m_failedLableStr;
    MxTable*          m_table;
    NavigationControl* m_elementList[MAX_QUICK_BACK_CTRL];
    quint8            m_currentPageNo;
    quint8            m_maximumPages;
    quint8            m_totalRecords;
    QString           m_totalCamera;

public:
    explicit MxQuickBckupReportTable(QWidget *parent = 0);

    BgTile*           m_insideQuickBkpTile;

    ~MxQuickBckupReportTable();


    void createDefaultComponent ();
    void showReport(QStringList reportList, quint8 totalCam, quint8 success, quint8 fail);
    void successCamera(quint8 sucess);
    void takeLeftKeyAction();
    void takeRightKeyAction();
    virtual void escKeyPressed(QKeyEvent *event);
    virtual void tabKeyPressed(QKeyEvent *event);
    virtual void backTab_KeyPressed(QKeyEvent *event);

    void showEvent(QShowEvent * event);

signals:
    //void sigCloseAlert();

public slots:

    //void slotUpadateCurrentElement (int index);
    //void slotButtonClick(int);

};

#endif
