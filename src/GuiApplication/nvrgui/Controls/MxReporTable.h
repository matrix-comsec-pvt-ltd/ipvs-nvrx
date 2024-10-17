#ifndef MXREPORTTABLE_H
#define MXREPORTTABLE_H

#include <QWidget>
#include "Controls/Rectangle.h"
#include "Controls/Heading.h"
#include "Controls/Closebuttton.h"
#include "Controls/ControlButton.h"
#include "Controls/TableCell.h"
#include "Controls/TextLabel.h"

typedef enum
{
    REPORT_TABLE_CLS_BTN,
    REPORT_TABLE_PREV_BTN,
    REPORT_TABLE_NEXT_BTN,
    MAX_REPORT_TABLE_CTRL
}REPORT_TABLE_CTRL_e;

typedef enum{

    MX_CMD_FIELDS_DETECTED_IP_ADDR,
    MX_CMD_FEILDS_AUTO_CNFG_STATUS,
    MX_CMD_FEILDS_CAM_INDEX,
    MX_CMD_FEILDS_CHANGED_IP_ADDR,
    MX_CMD_FEILDS_CNFG_FAIL_REASON,
    MAX_MX_CMD_AUTO_CFG_STATUS_RPRT
}MX_CMD_AUTO_CFG_STATUS_RPRT_e;

class MxReportTable : public QWidget
{
    Q_OBJECT
public:
    explicit MxReportTable(quint8 totalRows,
                           quint8 totalColumns,
                           quint8 rowsPerPage,
                           QString reportHeadingStr,
                           QList<quint16> colWidthList,
                           QStringList headingList,
                           QWidget *parent = 0,
                           quint8 pageNo = 1,
                           quint8 rowHeight = SCALE_HEIGHT(50));
    ~MxReportTable();
    void showReport(QStringList reportList);
    void updateReport();

signals:
    void sigClosePage(quint8);

public slots:
    void slotButtonClick(int index);

private:
    bool            m_isDispFromCurrPage;
    quint8          m_maxPages;
    quint8          m_maxPagesSupported;
    quint8          m_currentPage;
    quint8          m_totalRows;
    quint8          m_totalColumns;
    quint8          m_rowsPerPage;
    quint8          m_rowHeight;
    quint16         m_xParam;
    quint16         m_yParam;
    quint16         m_width;
    quint16         m_height;
    QStringList     m_reportList;
    QList<quint16>  m_columnWidth;
    Rectangle*      m_backGround;
    CloseButtton*   m_closeButton;
    Heading*        m_heading;
    TableCell***    m_tablePtr;
    TextLabel***    m_lablePtr;
    ControlButton*  m_prevBtn;
    ControlButton*  m_nextBtn;
};

#endif // MXREPORTTABLE_H
