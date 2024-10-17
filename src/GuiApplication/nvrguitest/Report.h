#ifndef REPORT_H
#define REPORT_H

#include <QWidget>
#include <QScrollArea>

#include "Controls/Closebuttton.h"
#include "Controls/ReadOnlyElement.h"
#include "Controls/Image.h"

typedef enum
{
    PREV_CTRL,
    NEXT_CTRL,
    CLOSE_CTRL,
    MAX_PAGE_CTRL
}PAGE_CTRL_e;

class Report : public QWidget
{
    Q_OBJECT
public:
    explicit Report(QWidget *parent = 0);
    ~Report();
    void createDefaultComponent();

signals:
    void sigCloseBtnClicked();
    
public slots:
    void slotCtrlClicked(int index);

private:
    int                 m_currentReportNo;
    int                 m_maxAvailableReport;
    QStringList         m_reportList;

    CloseButtton*       m_closeButton;
    ReadOnlyElement*    m_reportNo;
    Image*              m_prevBtn;
    Image*              m_nextBtn;
    TextLabel*          m_reportLabel;

    void changePage(int offset);
    void generateReports();
    void changeReportPage();
};

#endif // REPORT_H
