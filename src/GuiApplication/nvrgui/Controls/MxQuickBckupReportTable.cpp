#include "MxQuickBckupReportTable.h"
#include "../Layout/Layout.h"

#define QUICK_BKUP_RPT_MAIN_RECT_WIDTH          SCALE_WIDTH(1020)
#define QUICK_BKUP_RPT_MAIN_RECT_HEIGHT         SCALE_HEIGHT(555)

#define QUICK_BKUP_RPT_HEADER_RECT_WIDTH        SCALE_WIDTH(350)
#define QUICK_BKUP_RPT_HEADER_RECT_HEIGHT       SCALE_HEIGHT(45)
#define QUICK_BKUP_RPT_INSIDE_RECT_WIDTH        SCALE_WIDTH(950)
#define QUICK_BKUP_RPT_INSIDE_RECT_HEIGHT       SCALE_HEIGHT(420)
#define QUICK_BKUP_RPT_BGTILE_WIDTH             SCALE_WIDTH(1300 + 35 + 60)

#define QUICK_BKUP_RPT_HEADER_TEXT              "Quick Backup - Status"

typedef enum
{
    MX_TOTAL_CAMERA_FOUND,
    MX_SUCCESS_FOUND,
    MX_FAILED_FOUND,
    MX_TOTAL_CAMERAS_STR,
    MX_SUCCESS_STR,
    MX_FAILED_STR,
    MAX_MX_LABLE_QUICKBKUP
}MX_QUICK_BKUP_REPORT_TABLE_e;

static const QString quickBackupLableStr[] =
{
    "0",
    "0",
    "0",
    "Total Cameras",
    "Success",
    "Failed"
};

MxQuickBckupReportTable::MxQuickBckupReportTable(QWidget * parent)
                        :BackGround((parent->x() - SCALE_WIDTH(452) - (Layout::tvAdjustParam * LIVE_VIEW_STARTX)),
                        (parent->y() - SCALE_HEIGHT(90)),
                        QUICK_BKUP_RPT_MAIN_RECT_WIDTH ,
                        (QUICK_BKUP_RPT_MAIN_RECT_HEIGHT - QUICK_BKUP_RPT_HEADER_RECT_HEIGHT ),
                        BACKGROUND_TYPE_1,
                        MAX_TOOLBAR_BUTTON,
                        parent,
                        QUICK_BKUP_RPT_HEADER_RECT_WIDTH,
                        QUICK_BKUP_RPT_HEADER_RECT_HEIGHT,
                        QUICK_BKUP_RPT_HEADER_TEXT )
{
    m_currentPageNo = 0;
    m_maximumPages = 0;
    m_totalRecords = 0;

    createDefaultComponent();
    m_elementList[QUICK_BACK_CLOSE_BTN] = m_mainCloseButton;
    m_elementList[QUICK_BACK_TABLE_PAGE] = m_table;
    m_currElement = QUICK_BACK_CLOSE_BTN;
}

void MxQuickBckupReportTable::createDefaultComponent ()
{
    m_insideQuickBkpTile = new BgTile(((QUICK_BKUP_RPT_MAIN_RECT_WIDTH - QUICK_BKUP_RPT_INSIDE_RECT_WIDTH)/2),
                             ((QUICK_BKUP_RPT_MAIN_RECT_HEIGHT - QUICK_BKUP_RPT_INSIDE_RECT_HEIGHT)/2),
                             QUICK_BKUP_RPT_INSIDE_RECT_WIDTH,
                             QUICK_BKUP_RPT_INSIDE_RECT_HEIGHT + SCALE_HEIGHT(30),
                             COMMON_LAYER,
                             this);

   m_totalCameraFoundTextLableStr = new TextLabel((SCALE_WIDTH(35)+(QUICK_BKUP_RPT_INSIDE_RECT_WIDTH)/5),
                                          m_insideQuickBkpTile->y() + SCALE_HEIGHT(25),
                                          SCALE_FONT(30),
                                          quickBackupLableStr[MX_TOTAL_CAMERA_FOUND],
                                          this,
                                          NORMAL_FONT_COLOR,
                                          NORMAL_FONT_FAMILY,
                                          ALIGN_CENTRE_X_START_Y);

   m_sucess_TextLableStr = new TextLabel((SCALE_WIDTH(35)+(QUICK_BKUP_RPT_INSIDE_RECT_WIDTH)/2),
                                         m_insideQuickBkpTile->y() + SCALE_HEIGHT(25),
                                         SCALE_FONT(30),
                                         quickBackupLableStr[MX_SUCCESS_FOUND],
                                         this,
                                         NORMAL_FONT_COLOR,
                                         NORMAL_FONT_FAMILY,
                                         ALIGN_CENTRE_X_START_Y);

   m_failedTextLableStr = new TextLabel((SCALE_WIDTH(35)+(QUICK_BKUP_RPT_INSIDE_RECT_WIDTH)-(QUICK_BKUP_RPT_INSIDE_RECT_WIDTH/5)),
                                         m_insideQuickBkpTile->y()  + SCALE_HEIGHT(25) ,
                                         SCALE_FONT(30),
                                         quickBackupLableStr[MX_FAILED_FOUND],
                                         this,
                                         NORMAL_FONT_COLOR,
                                         NORMAL_FONT_FAMILY,
                                         ALIGN_CENTRE_X_START_Y);

   m_totalCameraLableStr = new TextLabel(SCALE_WIDTH(35)+(QUICK_BKUP_RPT_INSIDE_RECT_WIDTH)/5,
                                         m_totalCameraFoundTextLableStr->y() + SCALE_HEIGHT(55),
                                         NORMAL_FONT_SIZE,
                                         quickBackupLableStr[MX_TOTAL_CAMERAS_STR],
                                         this,
                                         NORMAL_FONT_COLOR,
                                         NORMAL_FONT_FAMILY,
                                         ALIGN_CENTRE_X_START_Y,
                                         0, 0, ((QUICK_BKUP_RPT_INSIDE_RECT_WIDTH / 3) - SCALE_WIDTH(15)));

   m_successLableStr = new TextLabel(  SCALE_WIDTH(35)+(QUICK_BKUP_RPT_INSIDE_RECT_WIDTH)/2,
                                         m_sucess_TextLableStr->y() + SCALE_HEIGHT(55),
                                         NORMAL_FONT_SIZE,
                                         quickBackupLableStr[MX_SUCCESS_STR],
                                         this,
                                         NORMAL_FONT_COLOR,
                                         NORMAL_FONT_FAMILY,
                                         ALIGN_CENTRE_X_START_Y,
                                        0, 0, ((QUICK_BKUP_RPT_INSIDE_RECT_WIDTH / 3) - SCALE_WIDTH(100)));

   m_failedLableStr = new TextLabel(SCALE_WIDTH(35)+(QUICK_BKUP_RPT_INSIDE_RECT_WIDTH)-(QUICK_BKUP_RPT_INSIDE_RECT_WIDTH/5),
                                         m_failedTextLableStr->y() + SCALE_HEIGHT(55),
                                         NORMAL_FONT_SIZE,
                                         quickBackupLableStr[MX_FAILED_STR],
                                         this,
                                         NORMAL_FONT_COLOR,
                                         NORMAL_FONT_FAMILY,
                                         ALIGN_CENTRE_X_START_Y,
                                        0, 0, ((QUICK_BKUP_RPT_INSIDE_RECT_WIDTH / 3) - SCALE_WIDTH(15)));

   QList<quint16> widthList;
   widthList << SCALE_WIDTH(430) << SCALE_WIDTH(140) << SCALE_WIDTH(363);
   QStringList headingLIst;
   headingLIst << "Camera" << "Status" << "Reason";
   m_table =new MxTable(m_insideQuickBkpTile->x() + SCALE_WIDTH(10) ,(m_insideQuickBkpTile->y() + SCALE_HEIGHT(123)) , 64,3,8,widthList,headingLIst,this,1,SCALE_HEIGHT(30), SCALE_WIDTH(5), SCALE_HEIGHT(5));
}

void MxQuickBckupReportTable::showReport(QStringList reportList, quint8 totalCam, quint8 success, quint8 fail)
{
    m_totalRecords  = (reportList.count()/4);
    m_totalCameraFoundTextLableStr->changeText(INT_TO_QSTRING(totalCam));
    m_sucess_TextLableStr->changeText(INT_TO_QSTRING(success));
    m_failedTextLableStr->changeText(INT_TO_QSTRING(fail));

    m_totalCameraFoundTextLableStr->update();
    m_sucess_TextLableStr->update();
    m_failedTextLableStr->update();

    m_table->showReport(reportList);
}

void MxQuickBckupReportTable::takeLeftKeyAction()
{
    do
    {
        m_currElement = (m_currElement - 1 + MAX_QUICK_BACK_CTRL) % MAX_QUICK_BACK_CTRL;

    }while((m_elementList[m_currElement] == NULL) || (!m_elementList[m_currElement]->getIsEnabled()));

    if(m_currElement == QUICK_BACK_TABLE_PAGE)
    {
        m_elementList[m_currElement]->forceFocusToPage(false);
    }
    else
    {
        m_elementList[m_currElement]->forceActiveFocus();
    }
}

void MxQuickBckupReportTable::takeRightKeyAction()
{
    do
    {
        m_currElement = (m_currElement + 1) % MAX_QUICK_BACK_CTRL;

    }while((m_elementList[m_currElement] == NULL) || (!m_elementList[m_currElement]->getIsEnabled()));

    if(m_currElement == QUICK_BACK_TABLE_PAGE)
    {
        m_elementList[m_currElement]->forceFocusToPage(true);
    }
    else
    {
        m_elementList[m_currElement]->forceActiveFocus();
    }
}

void MxQuickBckupReportTable::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    m_currElement = QUICK_BACK_CLOSE_BTN;
    m_elementList[m_currElement]->forceActiveFocus();
}

void MxQuickBckupReportTable::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void MxQuickBckupReportTable::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void MxQuickBckupReportTable::showEvent(QShowEvent *event)
{
    m_currElement = QUICK_BACK_CLOSE_BTN;
    m_elementList[m_currElement]->forceActiveFocus();
    QWidget::showEvent(event);
}

MxQuickBckupReportTable::~MxQuickBckupReportTable()
{
    DELETE_OBJ(m_table);
    DELETE_OBJ(m_failedLableStr);
    DELETE_OBJ(m_successLableStr);
    DELETE_OBJ(m_totalCameraLableStr);
    DELETE_OBJ(m_failedTextLableStr);
    DELETE_OBJ(m_sucess_TextLableStr);
    DELETE_OBJ(m_totalCameraFoundTextLableStr);
    DELETE_OBJ(m_insideQuickBkpTile);
}
