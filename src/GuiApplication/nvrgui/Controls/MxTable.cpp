#include "MxTable.h"



MxTable::MxTable(quint16 startX,
                 quint16 startY,
                 quint8 totalRows,
                 quint8 totalColumns,
                 quint8 rowsPerPage,
                 QList<quint16> colWidthList,
                 QStringList headingList,
                 QWidget *parent,
                 quint8 pageNo,
                 quint8 rowHeight,
                 quint16 xOffset,
                 quint16 yOffset) : KeyBoard(parent), NavigationControl(0, true),
     m_maxPagesSupported((((totalRows-1)/rowsPerPage)+1)), m_currentPage(pageNo), m_totalRows(totalRows), m_totalColumns(totalColumns),
     m_rowsPerPage(((rowsPerPage < totalRows) ? (rowsPerPage + 1) : (totalRows + 1))),
     m_rowHeight(rowHeight), m_xParam(startX), m_yParam(startY),
     m_xOffset(xOffset), m_yOffset(yOffset), m_colWidthList(colWidthList),
     m_headingList(headingList), m_maximumPages(8)
{
    m_elementList[QUICK_BKUP_RPT_CLOSE_PAGE] = NULL;

    INIT_OBJ(m_insideQuickBkpTile);
    INIT_OBJ(m_totalCameraFoundTextLableStr);

    m_isDispFromCurrPage = false;
    m_maxPages = 0;
    m_statrX = 0;
    m_startY = 0;

    //Allocation memory for table and text lables
    m_tablePtr = new TableCell**[m_totalColumns];
    m_lablePtr = new TextLabel**[m_totalColumns];

    quint8      colIndex;
    quint8      colCount;
    quint16     temp;
    if(IS_VALID_OBJ(m_tablePtr) && IS_VALID_OBJ(m_lablePtr))
    {
        for(colIndex=0; colIndex<m_totalColumns; colIndex++)
        {
            m_tablePtr[colIndex] = new TableCell*[m_rowsPerPage];
            m_lablePtr[colIndex] = new TextLabel*[m_rowsPerPage];
        }
    }

    //Calculating height and width for report page
    temp = 0;
    colCount = colWidthList.count();
    //Calculating report width from width list passes in constructor with padding 0f 60(30 from left and right)
    for(colIndex=0; colIndex<colCount; colIndex++)
    {
        temp += colWidthList.at(colIndex);
    }
    m_width = temp;
    //calculating report height using total rowsPerPage and padding of 110(50 at top &  60 at bottom)
    temp = m_rowsPerPage * m_rowHeight;
    m_height = temp;

    //fn is called here bcoz m_xParam and m_yParam going to changed in following fn
   // createDefaultElement ();
    quint8      rowIndex = 0;
    //quint8      colIndex;
    quint16     xParamCell = 0;
    quint16     yParamCell = 0;

     xParamCell =  m_xOffset;
    for(colIndex=0; colIndex<m_totalColumns; colIndex++)
    {
        yParamCell = m_yOffset;
        if(IS_VALID_OBJ(m_tablePtr[colIndex]) && IS_VALID_OBJ(m_lablePtr[colIndex]))
        {
            for(rowIndex=0; rowIndex<m_rowsPerPage; rowIndex++)
            {
                m_tablePtr[colIndex][rowIndex] = new TableCell(xParamCell,
                                                               yParamCell,
                                                               m_colWidthList.at(colIndex),
                                                               m_rowHeight,
                                                               this,
                                                               (rowIndex == 0) ? (true) : (false));

                m_lablePtr[colIndex][rowIndex] = new TextLabel((xParamCell + SCALE_WIDTH(10)),
                                                               (yParamCell + (m_rowHeight/2)),
                                                               NORMAL_FONT_SIZE,
                                                               ((rowIndex == 0) ? (m_headingList.at(colIndex)) : (QString(""))),
                                                               this,
                                                               NORMAL_FONT_COLOR,
                                                               NORMAL_FONT_FAMILY,
                                                               ALIGN_START_X_CENTRE_Y,
                                                               0, 0, (m_colWidthList.at(colIndex) - SCALE_WIDTH(10)));
                yParamCell += m_rowHeight;
            }
        }
        xParamCell += m_colWidthList.at(colIndex);
    }

//    m_xParam = (((parent->width())-m_width)/2) - 100;
//    m_yParam = (((parent->height())-m_height)/2) -100;

    this->setGeometry(m_xParam,m_yParam,xParamCell,yParamCell +SCALE_HEIGHT(45));
    m_firstPageBtn = new ControlButton(FIRSTPAGE_BUTTON_INDEX,
                                     SCALE_WIDTH(330) ,
                                     (this->height() - SCALE_HEIGHT(38))  ,
                                     BGTILE_LARGE_SIZE_WIDTH,
                                     BGTILE_HEIGHT,
                                     this, NO_LAYER, 0,
                                     "", false,
                                     QUICK_BKUP_RPT_FIRST_PAGE);


   m_elementList[QUICK_BKUP_RPT_FIRST_PAGE] = m_firstPageBtn;

   if(IS_VALID_OBJ(m_firstPageBtn))
   {
         connect(m_firstPageBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
         connect (m_firstPageBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));
    }

    m_prevPageBtn = new ControlButton(PREVIOUS_BUTTON_INDEX,
                                    (m_firstPageBtn->x () + m_firstPageBtn->width () + SCALE_WIDTH(15)),
                                    (this->height() - SCALE_HEIGHT(38)) ,
                                    BGTILE_LARGE_SIZE_WIDTH,
                                    BGTILE_HEIGHT,
                                    this, NO_LAYER, 0,
                                    "", false,
                                    QUICK_BKUP_RPT_PREV_PAGE);

    m_elementList[QUICK_BKUP_RPT_PREV_PAGE] = m_prevPageBtn;

    if(IS_VALID_OBJ(m_prevPageBtn))
    {
         connect(m_prevPageBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
         connect (m_prevPageBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));
    }
    INIT_OBJ(m_pageNoReadOnly);
    m_pageNoReadOnly = new ReadOnlyElement((m_prevPageBtn->x () + m_prevPageBtn->width () +SCALE_WIDTH(10)),
                                         (this->height() - SCALE_HEIGHT(35)) ,
                                         SCALE_WIDTH(40), SCALE_HEIGHT(32), SCALE_WIDTH(90), SCALE_HEIGHT(30),"",this,NO_LAYER);




    m_nextPageBtn = new ControlButton(NEXT_BUTTON_INDEX,
                                    (m_pageNoReadOnly->x () + m_pageNoReadOnly->width () + SCALE_WIDTH(10)),
                                    (this->height() - SCALE_HEIGHT(38)) ,
                                    BGTILE_LARGE_SIZE_WIDTH,
                                    BGTILE_HEIGHT,
                                    this, NO_LAYER, 0,
                                    "", false,
                                    QUICK_BKUP_RPT_NEXT_PAGE);

    m_elementList[QUICK_BKUP_RPT_NEXT_PAGE] = m_nextPageBtn;

    if(IS_VALID_OBJ(m_nextPageBtn))
    {
         connect(m_nextPageBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));

         connect (m_nextPageBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));
    }

    m_lastPageBtn = new ControlButton(LAST_BUTTON_INDEX,
                                    m_nextPageBtn->x ()+ m_nextPageBtn->width () + SCALE_WIDTH(15),
                                    (this->height() - SCALE_HEIGHT(38)) ,
                                    BGTILE_LARGE_SIZE_WIDTH,
                                    BGTILE_HEIGHT,
                                    this, NO_LAYER, 0,
                                    "", false,
                                    QUICK_BKUP_RPT_LAST_PAGE);

    m_elementList[QUICK_BKUP_RPT_LAST_PAGE] = m_lastPageBtn;

    if(IS_VALID_OBJ(m_lastPageBtn))
    {
          connect(m_lastPageBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
          connect (m_lastPageBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));
     }

    m_currElement = QUICK_BKUP_RPT_FIRST_PAGE;
    this->repaint();
    this->show();
}

void MxTable::updateNavigationControlStatus ()
{

    m_prevPageBtn->setIsEnabled (false);
    m_firstPageBtn->setIsEnabled (false);
    m_nextPageBtn->setIsEnabled (false);
    m_lastPageBtn->setIsEnabled (false);

    m_prevPageBtn->setIsEnabled ((m_currentPage != 1 ? true : false ));
    m_firstPageBtn->setIsEnabled ((m_currentPage != 1 ? true : false ));

    if( m_currentPage == (m_maxPages ))
    {
        m_nextPageBtn->setIsEnabled (false);
        m_lastPageBtn->setIsEnabled (false);
        m_currElement = QUICK_BKUP_RPT_PREV_PAGE;
    }
    else if( m_currentPage < (m_maximumPages ) )
    {
        m_nextPageBtn->setIsEnabled (true);
        m_lastPageBtn->setIsEnabled (true);
        if((m_currElement != QUICK_BKUP_RPT_PREV_PAGE) || (m_currentPage == 1))
            m_currElement = QUICK_BKUP_RPT_NEXT_PAGE;
    }

    m_elementList[m_currElement]->forceActiveFocus ();
}

void MxTable::showReport(QStringList reportList)
{
    quint8 totalRecords;
    m_reportList = reportList;
    totalRecords  = (m_reportList.count()/m_totalColumns);
    if(totalRecords <= ((m_currentPage-1)*(m_rowsPerPage-1)))
    {
        m_currentPage = ((totalRecords-1)/(m_rowsPerPage-1))+1;
    }
    updateReport();
}

void MxTable::updateReport()
{
    quint8  totalReport = 0;
    quint8  totalPages = 0;
    quint8  rowsToDisplay = 0;
    quint8  colIndex = 0;
    quint8  rowIndex = 0;
    quint16 firstRecIndex = 0;

    m_pageNoReadOnly->changeValue ((QString(" ") + QString("Page")+ QString(" %1").arg (m_currentPage )));
    if((m_reportList.count()) > 0)
    {
        totalReport = (((m_reportList.count()-1)/m_totalColumns)+1);
        totalPages = ((totalReport-1)/(m_rowsPerPage-1))+1;
        m_maxPages = ((totalPages <= m_maxPagesSupported) ? (totalPages) : (m_maxPagesSupported));
        rowsToDisplay = ((m_currentPage >= m_maxPages) ? (((totalReport-1)%(m_rowsPerPage-1))+1) : (m_rowsPerPage-1));
        firstRecIndex = ((m_currentPage-1)*(m_rowsPerPage-1)*(m_totalColumns)); //minus 1 in m_rowsPerPage is due to heading row
    }

    for(rowIndex=1; rowIndex<m_rowsPerPage; rowIndex++) //rowsIndex is 1 due to heading row
    {
        for(colIndex=0; colIndex<m_totalColumns; colIndex++)
        {
            if(IS_VALID_OBJ(m_lablePtr[colIndex][rowIndex]))
            {
                m_lablePtr[colIndex][rowIndex]->changeText((rowIndex <= rowsToDisplay) ? (m_reportList.at(firstRecIndex+((rowIndex-1)*m_totalColumns)+colIndex)) : (""));
                m_lablePtr[colIndex][rowIndex]->repaint();
            }
        }
    }

    updateNavigationControlStatus();
}

bool MxTable::takeLeftKeyAction()
{
    bool status = true;

    do
    {
        m_currElement = (m_currElement - 1 + MAX_QUICK_BKUP_ELEMENTS) % MAX_QUICK_BKUP_ELEMENTS;
        if(m_currElement == QUICK_BKUP_RPT_CLOSE_PAGE)
        {
            status = false;
            break;
        }

    }while((m_elementList[m_currElement] == NULL) || (!m_elementList[m_currElement]->getIsEnabled()));

    if(status == true)
    {
        m_elementList[m_currElement]->forceActiveFocus();
    }

    return status;
}

bool MxTable::takeRightKeyAction()
{
    bool status = true;

    do
    {
        m_currElement = (m_currElement + 1) % MAX_QUICK_BKUP_ELEMENTS;
        if(m_currElement == QUICK_BKUP_RPT_CLOSE_PAGE)
        {
            status = false;
            break;
        }

    }while((m_elementList[m_currElement] == NULL) || (!m_elementList[m_currElement]->getIsEnabled()));

    if(status == true)
    {
        m_elementList[m_currElement]->forceActiveFocus();
    }

    return status;
}

void MxTable::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    if(takeRightKeyAction() == false)
    {
        QWidget::keyPressEvent(event);
    }
}

void MxTable::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    if(takeLeftKeyAction() == false)
    {
        QWidget::keyPressEvent(event);
    }
}

void MxTable::forceFocusToPage(bool isFirstElem)
{
    m_currElement = QUICK_BKUP_RPT_CLOSE_PAGE;
    if(isFirstElem)
    {
        takeRightKeyAction();
    }
    else
    {
        takeLeftKeyAction();
    }
}

MxTable::~MxTable()
{
    quint8  colIndex;
    quint8  rowIndex;

    for(colIndex=0; colIndex<m_totalColumns; colIndex++)
    {
        for(rowIndex=0; rowIndex<m_rowsPerPage; rowIndex++)
        {
            DELETE_OBJ(m_tablePtr[colIndex][rowIndex]);
            DELETE_OBJ(m_lablePtr[colIndex][rowIndex]);
        }
    }

    for(colIndex=0; colIndex<m_totalColumns; colIndex++)
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

    if(IS_VALID_OBJ(m_firstPageBtn))
    {
        disconnect(m_firstPageBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
        disconnect (m_firstPageBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
        DELETE_OBJ (m_firstPageBtn);
    }

    if(IS_VALID_OBJ(m_prevPageBtn))
    {
        disconnect(m_prevPageBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
        disconnect (m_prevPageBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
        DELETE_OBJ (m_prevPageBtn);
    }

    if(IS_VALID_OBJ(m_nextPageBtn))
    {
        disconnect(m_nextPageBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
        disconnect (m_nextPageBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
        DELETE_OBJ (m_nextPageBtn);
    }

    if(IS_VALID_OBJ(m_lastPageBtn))
    {
        disconnect(m_lastPageBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
        disconnect (m_lastPageBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
        DELETE_OBJ (m_lastPageBtn);
    }
    DELETE_OBJ (m_pageNoReadOnly);
}

void MxTable::slotUpadateCurrentElement (int index)
{
    m_currElement = index;
}

void MxTable::slotButtonClick (int index)
{
    switch(index)
    {
    case QUICK_BKUP_RPT_FIRST_PAGE:
        m_currentPage = 1;
        updateReport();
        break;

    case QUICK_BKUP_RPT_PREV_PAGE:
        if(m_currentPage > 0)
        {
            m_currentPage --;
        }
        updateReport();

        break;

    case QUICK_BKUP_RPT_NEXT_PAGE:

        if (m_currentPage != (m_maxPages  ))
        {
             m_currentPage ++;
        }
        updateReport();
        break;

    case QUICK_BKUP_RPT_LAST_PAGE:
        m_currentPage =( m_maxPages );
        updateReport();
        break;

    default:
        break;
    }
}

