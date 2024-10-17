#include "MxReporTable.h"

MxReportTable::MxReportTable(quint8 totalRows,
                             quint8 totalColumns,
                             quint8 rowsPerPage,
                             QString reportHeadingStr,
                             QList<quint16> colWidthList,
                             QStringList headingList,
                             QWidget *parent,
                             quint8 pageNo,
                             quint8 rowHeight)
    : QWidget(parent), m_maxPagesSupported((((totalRows-1)/rowsPerPage)+1)), m_currentPage(pageNo), m_totalRows(totalRows),
      m_totalColumns(totalColumns), m_rowsPerPage(((rowsPerPage < totalRows) ? (rowsPerPage + 1) : (totalRows + 1))), m_rowHeight(rowHeight)
{
    quint8 colIndex, rowIndex, colCount;
    quint16 marginCalc = 0, xParamCell, yParamCell;

    m_isDispFromCurrPage = false;
    m_maxPages = 0;
    m_columnWidth = colWidthList;
    /* Allocation memory for table and text lables */
    m_tablePtr = new TableCell**[m_totalColumns];
    m_lablePtr = new TextLabel**[m_totalColumns];

    for(colIndex = 0; colIndex < m_totalColumns; colIndex++)
    {
        m_tablePtr[colIndex] = new TableCell*[m_rowsPerPage];
        m_lablePtr[colIndex] = new TextLabel*[m_rowsPerPage];
    }

    //Calculating height and width for report page
    colCount = m_columnWidth.count();

    //Calculating report width from width list passes in constructor with padding 0f 60(30 from left and right)
    for(colIndex = 0; colIndex < colCount; colIndex++)
    {
        marginCalc += m_columnWidth.at(colIndex);
    }
    m_width = marginCalc + SCALE_WIDTH(60);

    //calculating report height using total rowsPerPage and padding of 110(50 at top &  60 at bottom)
    marginCalc = m_rowsPerPage * m_rowHeight;
    m_height = marginCalc + SCALE_HEIGHT(110);

    m_xParam = (((parent->width())-m_width)/2);
    m_yParam = (((parent->height())-m_height)/2);

    this->setGeometry(parent->geometry());

    m_backGround = new Rectangle(m_xParam,
                                 m_yParam,
                                 m_width,
                                 m_height,
                                 0,
                                 NORMAL_BKG_COLOR,
                                 NORMAL_BKG_COLOR,
                                 this);

    m_closeButton = new CloseButtton ((m_backGround->x () + m_backGround->width () - SCALE_WIDTH(20)),
                                      (m_backGround->y () + SCALE_HEIGHT(20)),
                                      this,
                                      CLOSE_BTN_TYPE_1,
                                      REPORT_TABLE_CLS_BTN);
    connect (m_closeButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    m_heading = new Heading( (m_backGround->x() + (m_backGround->width ()/2)),
                             (m_backGround->y() + SCALE_HEIGHT(25)),
                             reportHeadingStr,
                             this,
                             HEADING_TYPE_2);

    //creating cell and textlable
    xParamCell = m_backGround->x() + SCALE_WIDTH(30);
    for(colIndex = 0; colIndex < m_totalColumns; colIndex++)
    {
        yParamCell = m_backGround->y() + SCALE_HEIGHT(50);
        for(rowIndex = 0; rowIndex < m_rowsPerPage; rowIndex++)
        {
            m_tablePtr[colIndex][rowIndex] = new TableCell(xParamCell,
                                                           yParamCell,
                                                           m_columnWidth.at(colIndex),
                                                           m_rowHeight,
                                                           this,
                                                           (rowIndex == 0) ? (true) : (false));

            m_lablePtr[colIndex][rowIndex] = new TextLabel((xParamCell + SCALE_WIDTH(10)),
                                                           (yParamCell + (m_rowHeight/2)),
                                                           NORMAL_FONT_SIZE,
                                                           ((rowIndex == 0) ? (headingList.at(colIndex)) : (QString(""))),
                                                           this,
                                                           NORMAL_FONT_COLOR,
                                                           NORMAL_FONT_FAMILY,
                                                           ALIGN_START_X_CENTRE_Y,
                                                           0, 0, (m_columnWidth.at(colIndex) - SCALE_WIDTH(10)));
            yParamCell += m_rowHeight;
        }
        xParamCell += m_columnWidth.at(colIndex);
    }

    m_prevBtn = new ControlButton(PREVIOUS_BUTTON_INDEX,
                                  (m_backGround->x() + SCALE_WIDTH(26)),
                                  (m_tablePtr[m_totalColumns-1][m_rowsPerPage-1]->y() + m_rowHeight + SCALE_HEIGHT(5)),
                                  SCALE_WIDTH(100),
                                  BGTILE_HEIGHT,
                                  this,
                                  NO_LAYER,
                                  0,
                                  "Prev",
                                  true,
                                  REPORT_TABLE_PREV_BTN,
                                  false);
    connect (m_prevBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));
    m_prevBtn->setVisible(false);

    m_nextBtn = new ControlButton(NEXT_BUTTON_INDEX,
                                  (m_backGround->x() + (m_width - SCALE_WIDTH(102))),
                                  (m_tablePtr[m_totalColumns-1][m_rowsPerPage-1]->y() + m_rowHeight + SCALE_HEIGHT(5)),
                                  SCALE_WIDTH(100),
                                  BGTILE_HEIGHT,
                                  this,
                                  NO_LAYER,
                                  0,
                                  "Next",
                                  true,
                                  REPORT_TABLE_NEXT_BTN);
    connect (m_nextBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));
    m_nextBtn->setVisible(false);

    this->show();
}

MxReportTable::~MxReportTable()
{
    quint8 colIndex, rowIndex;

    DELETE_OBJ(m_backGround);
    disconnect (m_closeButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    DELETE_OBJ(m_closeButton);
    DELETE_OBJ(m_heading);

    for(colIndex = 0; colIndex < m_totalColumns; colIndex++)
    {
        for(rowIndex = 0; rowIndex < m_rowsPerPage; rowIndex++)
        {
            DELETE_OBJ(m_tablePtr[colIndex][rowIndex]);
            DELETE_OBJ(m_lablePtr[colIndex][rowIndex]);
        }
    }

    for(colIndex = 0; colIndex < m_totalColumns; colIndex++)
    {
        delete[] m_tablePtr[colIndex];
        delete[] m_lablePtr[colIndex];
    }

    if (NULL != m_tablePtr)
    {
        delete[] m_tablePtr;
    }

    if (NULL != m_lablePtr)
    {
        delete[] m_lablePtr;
    }

    connect (m_prevBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));
    DELETE_OBJ(m_prevBtn);

    connect (m_nextBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));
    DELETE_OBJ(m_nextBtn);
}

void MxReportTable::showReport(QStringList reportList)
{
    quint8 totalRecords;

    m_reportList = reportList;
    totalRecords = (m_reportList.count()/m_totalColumns);

    if(totalRecords <= ((m_currentPage-1)*(m_rowsPerPage-1)))
    {
        m_currentPage = ((totalRecords-1)/(m_rowsPerPage-1))+1;
    }

    if(totalRecords > (m_rowsPerPage-1))
    {
        m_nextBtn->setVisible(true);
        m_prevBtn->setVisible(true);
    }
    else
    {
        m_nextBtn->setVisible(false);
        m_prevBtn->setVisible(false);
    }
    updateReport();
}

void MxReportTable::updateReport()
{
    quint8  totalReport = 0;
    quint8  totalPages = 0;
    quint8  rowsToDisplay = 0;
    quint8  colIndex = 0;
    quint8  rowIndex = 0;
    quint16 firstRecIndex = 0;

    if((m_reportList.count()) > 0)
    {
        totalReport = (((m_reportList.count()-1)/m_totalColumns)+1);
        totalPages = ((totalReport-1)/(m_rowsPerPage-1))+1;
        m_maxPages = ((totalPages <= m_maxPagesSupported) ? (totalPages) : (m_maxPagesSupported));

        m_nextBtn->setIsEnabled((m_currentPage >= m_maxPages) ? false : true);
        m_nextBtn->update();

        m_prevBtn->setIsEnabled((m_currentPage <= 1) ? false : true);
        m_prevBtn->update();

        rowsToDisplay = ((m_currentPage >= m_maxPages) ? (((totalReport-1)%(m_rowsPerPage-1))+1) : (m_rowsPerPage-1));
        firstRecIndex = ((m_currentPage-1)*(m_rowsPerPage-1)*(m_totalColumns)); //minus 1 in m_rowsPerPage is due to heading row
    }

    for(rowIndex = 1; rowIndex < m_rowsPerPage; rowIndex++) //rowsIndex is 1 due to heading row
    {
        for(colIndex = 0; colIndex < m_totalColumns; colIndex++)
        {
            m_lablePtr[colIndex][rowIndex]->changeText(((rowIndex <= rowsToDisplay) ?
                                                            (m_reportList.at(firstRecIndex+((rowIndex-1)*m_totalColumns)+colIndex)) :
                                                            ("")), (m_columnWidth.at(colIndex) - SCALE_WIDTH(10)));
            m_lablePtr[colIndex][rowIndex]->update();
        }
    }
}

void MxReportTable::slotButtonClick (int index)
{
    switch(index)
    {
        case REPORT_TABLE_CLS_BTN:
        {
            emit sigClosePage(m_currentPage);
        }
        break;

        case REPORT_TABLE_NEXT_BTN:
        {
            if (m_currentPage < m_maxPages)
            {
                m_currentPage++;
                updateReport();
            }
        }
        break;

        case REPORT_TABLE_PREV_BTN:
        {
            if(m_currentPage > 1)
            {
                m_currentPage--;
                updateReport();
            }
        }
        break;

        default:
        {
            /* Nothing to do */
        }
        break;
    }
}
