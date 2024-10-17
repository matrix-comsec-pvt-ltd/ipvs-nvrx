#include "Report.h"
#include "FileIO.h"
#include "HardwareTestControl.h"

#define PREV_BTN_IMAGE_PATH     IMAGE_PATH"/ControlButtons/PreviousButton_1/"
#define NEXT_BTN_IMAGE_PATH     IMAGE_PATH"/ControlButtons/NextButton_1/"

Report::Report(QWidget *parent):QWidget(parent)
{
    this->setGeometry(0,0,parent->width(),parent->height());

    createDefaultComponent();
    generateReports();
    changePage(0);
    this->show();
}

void Report::createDefaultComponent()
{
    m_closeButton = new CloseButtton((this->width() - 30),
                                     (30),
                                     this,
                                     CLOSE_BTN_TYPE_1,
                                     CLOSE_CTRL);    

    connect(m_closeButton,
            SIGNAL(sigButtonClick(int)),
            this,
            SLOT(slotCtrlClicked(int)));

    m_reportNo = new ReadOnlyElement(((this->width() - 120)/2),
                                     (this->height() - 60),
                                     0,
                                     30,
                                     60,
                                     30,
                                     QString("1"),
                                     this,
                                     NO_LAYER);

    m_prevBtn = new Image((m_reportNo->x()),
                          (m_reportNo->y()),
                          QString(PREV_BTN_IMAGE_PATH),
                          this,
                          END_X_START_Y,
                          PREV_CTRL);
    connect(m_prevBtn,
            SIGNAL(sigImageClicked(int)),
            this,
            SLOT(slotCtrlClicked(int)));

    m_nextBtn = new Image((m_reportNo->x() + m_reportNo->width()),
                          (m_reportNo->y()),
                          QString(NEXT_BTN_IMAGE_PATH),
                          this,
                          START_X_START_Y,
                          NEXT_CTRL,
                          true);
    connect(m_nextBtn,
            SIGNAL(sigImageClicked(int)),
            this,
            SLOT(slotCtrlClicked(int)));

    m_reportLabel = new TextLabel(20,0,
                                  18,
                                  "",
                                  this,
                                  QString(MAIN_HEADING_FONT_COLOR),
                                  WINDOW_FONT_FAMILY,
                                  ALIGN_START_X_START_Y);
    this->show();
}

void Report::generateReports()
{
    FileIO*         m_fileIO;
    QString         data;
    QString         filePath = HW_TEST_REPORT_PATH + QString(HardwareTestControl::boardTypeString).replace(' ', '_')
                                + QString("_") + QString(HardwareTestControl::deviceSerialNumber) + QString(".txt");
    QVector <QStringRef> reportList;   

    m_fileIO = new FileIO(this,filePath);
    data = m_fileIO->read();

    reportList = data.splitRef("*************************");

    m_maxAvailableReport = reportList.length() - 1;
    m_currentReportNo = ((m_maxAvailableReport < 1) ? (0) : (1));

    m_reportList.clear();
    for(quint8 loop = 0; loop < m_maxAvailableReport; loop++)
    {
        m_reportList << reportList.at(loop).toString();
    }
}

void Report::changeReportPage()
{
    if((m_currentReportNo >= 1) && (m_currentReportNo <= m_maxAvailableReport))
    {
        m_reportLabel->changeText(m_reportList.at(m_maxAvailableReport - m_currentReportNo));
        m_reportLabel->update();
    }
}

void Report::slotCtrlClicked(int index)
{
    switch (index)
    {
    case PREV_CTRL:
        changePage(-1);
        break;

    case NEXT_CTRL:
        changePage(1);
        break;

    case CLOSE_CTRL:
        emit sigCloseBtnClicked();
        break;

    default:
        break;
    }
}

void Report::changePage(int offset)
{
    m_currentReportNo += offset;
    bool isPrevPageAvailable = (m_currentReportNo <= 1) ? (false) : (true);
    bool isNextPageAvailable = (m_currentReportNo >= m_maxAvailableReport) ? (false) : (true);
    m_prevBtn->setIsEnabled(isPrevPageAvailable);
    m_nextBtn->setIsEnabled(isNextPageAvailable);
    if (m_maxAvailableReport > 1)
    {
        if (false == isPrevPageAvailable)
        {
            m_nextBtn->forceActiveFocus();
        }

        if (false == isNextPageAvailable)
        {
            m_prevBtn->forceActiveFocus();
        }
    }
    m_reportNo->changeValue(QString("%1").arg(m_currentReportNo));
    changeReportPage();
}

Report::~Report()
{
    if(IS_VALID_OBJ(m_closeButton))
    {
        disconnect(m_closeButton,
                   SIGNAL(sigButtonClick(int)),
                   this,
                   SLOT(slotCtrlClicked(int)));
        DELETE_OBJ(m_closeButton);
    }

    DELETE_OBJ(m_reportNo);

    if(IS_VALID_OBJ(m_prevBtn))
    {
        disconnect(m_prevBtn,
                SIGNAL(sigImageClicked(int)),
                this,
                SLOT(slotCtrlClicked(int)));
        DELETE_OBJ(m_prevBtn);
    }

    if(IS_VALID_OBJ(m_nextBtn))
    {
        disconnect(m_nextBtn,
                SIGNAL(sigImageClicked(int)),
                this,
                SLOT(slotCtrlClicked(int)));
        DELETE_OBJ(m_nextBtn);
    }    

    DELETE_OBJ(m_reportLabel);
}
