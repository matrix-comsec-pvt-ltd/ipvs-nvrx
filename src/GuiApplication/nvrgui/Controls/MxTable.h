#ifndef MXTABLE_H
#define MXTABLE_H

#include <QWidget>
#include <QKeyEvent>
#include "Controls/Rectangle.h"
#include "Controls/Heading.h"
#include "Controls/Closebuttton.h"
#include "Controls/ControlButton.h"
#include "Controls/TableCell.h"
#include "Controls/TextLabel.h"
#include "Controls/ReadOnlyElement.h"
#include "NavigationControl.h"


typedef enum
{
    QUICK_BKUP_RPT_CLOSE_PAGE,
    QUICK_BKUP_RPT_FIRST_PAGE,
    QUICK_BKUP_RPT_PREV_PAGE,
    QUICK_BKUP_RPT_NEXT_PAGE,
    QUICK_BKUP_RPT_LAST_PAGE,

    MAX_QUICK_BKUP_ELEMENTS
}QUICK_BKUP_REPORT_TABLE_ELEMENTS_e;

class MxTable : public KeyBoard, public NavigationControl
{
    Q_OBJECT
public:
    explicit MxTable(quint16 startX,
                     quint16 startY,
                     quint8 totalRows,
                     quint8 totalColumns,
                     quint8 rowsPerPage,
                     QList<quint16> colWidthList,
                     QStringList headingList,
                     QWidget *parent = 0,
                     quint8 pageNo = 1,
                     quint8 rowHeight = SCALE_HEIGHT(8),
                     quint16 xOffset = SCALE_WIDTH(10),
                     quint16 yOffset = SCALE_HEIGHT(50));
    ~ MxTable();
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
    quint16         m_xOffset;
    quint16         m_yOffset;
    quint8          m_statrX;
    quint8          m_startY;
    TableCell***    m_tablePtr;
    TextLabel***    m_lablePtr;
    QList<quint16>  m_colWidthList;
    QStringList     m_headingList;
    ControlButton*  m_firstPageBtn;
    ControlButton*  m_prevPageBtn;
    ControlButton*  m_nextPageBtn;
    ControlButton*  m_lastPageBtn;
    ReadOnlyElement*  m_pageNoReadOnly;
    NavigationControl* m_elementList[MAX_QUICK_BKUP_ELEMENTS];
    QStringList     m_reportList;
    quint8          m_currElement;
    quint8          m_maximumPages;
    TextLabel*      m_totalCameraFoundTextLableStr;
public:
    BgTile*           m_insideQuickBkpTile;
    void createDefaultElement();
    void updateNavigationControlStatus();
    void showReport(QStringList reportList);
    void updateReport();
    bool takeLeftKeyAction();
    bool takeRightKeyAction();
    virtual void tabKeyPressed(QKeyEvent *event);
    virtual void backTab_KeyPressed(QKeyEvent *event);
    void forceFocusToPage(bool isFirstElem);

public:
    void sigCloseAlert();
public slots:
    void slotUpadateCurrentElement (int index);
    void slotButtonClick(int);

};
#endif
