#include "CamSearchFailReport.h"
#include <QKeyEvent>
#include <QPainter>

#define CAM_SERCH_FAIL_WIDTH        SCALE_WIDTH(475)
#define CAM_SERCH_FAIL_HEIGHT       SCALE_HEIGHT(480)
#define SRNO_WIDTH                  SCALE_WIDTH(45)
#define IP_ADDR_WIDTH               150
#define STATUS_WIDTH                230

static const QString CamSearchFailReportStrings[]=
{
    "Failure Report",
    "Sr." + QString(" "),
    "IP Address",
    "Status",
    "Prev",
    "Next",
    "OK",
    "Cancel"
};

CamSearchFailReport::CamSearchFailReport(QStringList ipAddrList,
                                         QStringList failStatus,
                                         QWidget *parent) :
    KeyBoard(parent), ipAddressList(ipAddrList), failureStatusList(failStatus)
{
    this->setGeometry (0,0,parent->width (),parent->height ());
    createDefaultComponents();
}

CamSearchFailReport::~CamSearchFailReport ()
{
    delete backGround;

    disconnect (closeButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));

    disconnect (closeButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete closeButton;

    delete heading;

    for(quint8 index = 0 ; index < MAX_FAIL_RECORD_DATA; index++)
    {
        delete srNumber[index];
        delete srNumberStr[index];
        delete ipAddress[index];
        delete ipAddressStr[index];
        delete failureStatus[index];
        delete failureStatusStr[index];
    }

    disconnect (prevButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));

    disconnect (prevButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete prevButton;

    disconnect (nextButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));

    disconnect (nextButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete nextButton;
}

void CamSearchFailReport::createDefaultComponents ()
{
    totalReport = ipAddressList.length ();

    maximumPages = (totalReport % MAX_REPORT_ON_PAGE == 0 )?
                (totalReport /MAX_REPORT_ON_PAGE) :
                ((totalReport /MAX_REPORT_ON_PAGE) + 1);

    backGround = new Rectangle((SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH) + SCALE_WIDTH(20) + ((SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) - CAM_SERCH_FAIL_WIDTH) / 2)),
                               (SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT) + ((SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)- CAM_SERCH_FAIL_HEIGHT) / 2)),
                               CAM_SERCH_FAIL_WIDTH,
                               CAM_SERCH_FAIL_HEIGHT,
                               0,
                               NORMAL_BKG_COLOR,
                               NORMAL_BKG_COLOR,
                               this);

    closeButton = new CloseButtton ((backGround->x () + backGround->width () - SCALE_WIDTH(20)),
                                    (backGround->y () + SCALE_HEIGHT(20)),
                                    this,
                                    CLOSE_BTN_TYPE_1,
                                    CAM_SRCH_CLS_BTN);

    m_elementlist[CAM_SRCH_CLS_BTN] = closeButton;

    connect (closeButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    connect (closeButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    heading = new Heading(backGround->x () + backGround->width ()/2,
                          backGround->y () + SCALE_HEIGHT(20),
                          CamSearchFailReportStrings[0],
                          this,
                          HEADING_TYPE_2);

    srNumber[0] = new TableCell(backGround->x () + SCALE_WIDTH(23),
                                backGround->y () + SCALE_HEIGHT(50),
                                SRNO_WIDTH,
                                SCALE_HEIGHT(50),
                                this,
                                true);

    srNumberStr[0] = new TextLabel(srNumber[0]->x () + SCALE_WIDTH(10),
                                   srNumber[0]->y () +
                                   (srNumber[0]->height ())/2,
                                   NORMAL_FONT_SIZE,
                                   CamSearchFailReportStrings[1],
                                   this,
                                   NORMAL_FONT_COLOR,
                                   NORMAL_FONT_FAMILY,
                                   ALIGN_START_X_CENTRE_Y, 0, 0, SRNO_WIDTH);

    ipAddress[0] = new TableCell(srNumber[0]->x () +
                                 srNumber[0]->width (),
                                 srNumber[0]->y (),
                                 SCALE_WIDTH(IP_ADDR_WIDTH)-1,
                                 SCALE_HEIGHT(50),
                                 this,
                                 true);

    ipAddressStr[0] = new TextLabel(ipAddress[0]->x () + SCALE_WIDTH(10),
                                    ipAddress[0]->y () +
                                    (ipAddress[0]->height ())/2,
                                    NORMAL_FONT_SIZE,
                                    CamSearchFailReportStrings[2],
                                    this,
                                    NORMAL_FONT_COLOR,
                                    NORMAL_FONT_FAMILY,
                                    ALIGN_START_X_CENTRE_Y, 0, 0, SCALE_WIDTH(IP_ADDR_WIDTH)-1);

    failureStatus[0] = new TableCell(ipAddress[0]->x () +
                                     ipAddress[0]->width (),
                                     ipAddress[0]->y (),
                                     SCALE_WIDTH(STATUS_WIDTH)-1,
                                     SCALE_HEIGHT(50),
                                     this,
                                     true);

    failureStatusStr[0] = new TextLabel(failureStatus[0]->x () + SCALE_WIDTH(10),
                                        failureStatus[0]->y () +
                                        (failureStatus[0]->height ())/2,
                                        NORMAL_FONT_SIZE,
                                        CamSearchFailReportStrings[3],
                                        this,
                                        NORMAL_FONT_COLOR,
                                        NORMAL_FONT_FAMILY,
                                        ALIGN_START_X_CENTRE_Y, 0, 0, SCALE_WIDTH(STATUS_WIDTH)-1);


    for(quint8 index = 1 ; index < MAX_FAIL_RECORD_DATA; index++)
    {

        srNumber[index] = new TableCell(srNumber[(index -1)]->x (),
                                        srNumber[(index -1)]->y () +
                                        srNumber[(index -1)]->height (),
                                        srNumber[(index -1)]->width () - 1,
                                        BGTILE_HEIGHT,
                                        this);

        srNumberStr[index] = new TextLabel(srNumber[index]->x () + SCALE_WIDTH(10),
                                           srNumber[index]->y () +
                                           (srNumber[index]->height ())/2,
                                           NORMAL_FONT_SIZE,
                                           "",
                                           this,
                                           NORMAL_FONT_COLOR,
                                           NORMAL_FONT_FAMILY,
                                           ALIGN_START_X_CENTRE_Y, 0, 0, srNumber[(index -1)]->width () - 1);

        ipAddress[index] = new TableCell(ipAddress[(index -1)]->x (),
                                         ipAddress[(index -1)]->y () +
                                         ipAddress[(index -1)]->height (),
                                         ipAddress[(index -1)]->width () - 1,
                                         BGTILE_HEIGHT,
                                         this);

        ipAddressStr[index] = new TextLabel(ipAddress[index]->x () + SCALE_WIDTH(10),
                                            ipAddress[index]->y () +
                                            (ipAddress[index]->height ())/2,
                                            NORMAL_FONT_SIZE,
                                            "",
                                            this,
                                            NORMAL_FONT_COLOR,
                                            NORMAL_FONT_FAMILY,
                                            ALIGN_START_X_CENTRE_Y, 0, 0, ipAddress[(index -1)]->width () - 1);

        failureStatus[index] = new TableCell(failureStatus[(index -1)]->x (),
                                             failureStatus[(index -1)]->y () +
                                             failureStatus[(index -1)]->height (),
                                             failureStatus[(index -1)]->width () - 1,
                                             BGTILE_HEIGHT,
                                             this);

        failureStatusStr[index] = new TextLabel(failureStatus[index]->x () + SCALE_WIDTH(10),
                                                failureStatus[index]->y () +
                                                (failureStatus[index]->height ())/2,
                                                NORMAL_FONT_SIZE,
                                                "",
                                                this,
                                                NORMAL_FONT_COLOR,
                                                NORMAL_FONT_FAMILY,
                                                ALIGN_START_X_CENTRE_Y, 0, 0, failureStatus[(index -1)]->width () - 1);

    }

    prevButton = new ControlButton(PREVIOUS_BUTTON_INDEX,
                                   srNumber[0]->x () - SCALE_WIDTH(5),
                                   failureStatus[(MAX_FAIL_RECORD_DATA-1)]->y () +
                                   failureStatus[(MAX_FAIL_RECORD_DATA-1)]->height (),
                                   CAM_SERCH_FAIL_WIDTH - SCALE_WIDTH(60),
                                   BGTILE_HEIGHT,
                                   this,
                                   NO_LAYER,
                                   0,
                                   CamSearchFailReportStrings[4],
                                   false,
                                   CAM_SRCH_PREV_BTN,
                                   false);

    m_elementlist[CAM_SRCH_PREV_BTN] = prevButton;

    connect (prevButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    connect (prevButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    nextButton = new ControlButton(NEXT_BUTTON_INDEX,
                                   prevButton->x () +
                                   CAM_SERCH_FAIL_WIDTH - SCALE_WIDTH(110),
                                   failureStatus[(MAX_FAIL_RECORD_DATA-1)]->y () +
                                   failureStatus[(MAX_FAIL_RECORD_DATA-1)]->height (),
                                   BGTILE_MEDIUM_SIZE_WIDTH,
                                   BGTILE_HEIGHT,
                                   this,
                                   NO_LAYER,
                                   0,
                                   CamSearchFailReportStrings[5],
                                   true,
                                   CAM_SRCH_NEXT_BTN);

    m_elementlist[CAM_SRCH_NEXT_BTN] = nextButton;

    connect (nextButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    connect (nextButton,
             SIGNAL(sigUpdateCurrentElement(int)),
             this,
             SLOT(slotUpdateCurrentElement(int)));

    if(totalReport <= MAX_REPORT_ON_PAGE)
    {
        prevButton->setVisible (false);
        nextButton->setVisible (false);
    }

    currentPageNum = 0;
    showReports ();

    currElement = CAM_SRCH_CLS_BTN;
    m_elementlist[currElement]->forceActiveFocus ();
    this->show ();
}

void CamSearchFailReport::showReports ()
{
    quint8 fieldsOnPage;
    if( totalReport < (MAX_REPORT_ON_PAGE*(currentPageNum + 1)))
    {
        fieldsOnPage = totalReport - ((MAX_REPORT_ON_PAGE*(currentPageNum)) );
    }
    else
    {
        fieldsOnPage = MAX_REPORT_ON_PAGE;
    }

    for(quint8 index = 0 ; index < fieldsOnPage; index++)
    {
        srNumberStr[(index + 1)]->changeText (QString("%1").arg (index + (MAX_REPORT_ON_PAGE*(currentPageNum)) + 1));
        srNumberStr[(index + 1)]->update ();

        ipAddressStr[(index + 1)]->changeText (ipAddressList.at (index + (MAX_REPORT_ON_PAGE*(currentPageNum))));
        ipAddressStr[(index + 1)]->update ();

        failureStatusStr[(index + 1)]->changeText (failureStatusList.at (index + (MAX_REPORT_ON_PAGE*(currentPageNum))));
        failureStatusStr[(index + 1)]->update ();
    }

    for(quint8 index = fieldsOnPage ; index < MAX_REPORT_ON_PAGE; index++)
    {
        srNumberStr[(index + 1)]->changeText ("");
        srNumberStr[(index + 1)]->update ();

        ipAddressStr[(index + 1)]->changeText ("");
        ipAddressStr[(index + 1)]->update ();

        failureStatusStr[(index + 1)]->changeText ("");
        failureStatusStr[(index + 1)]->update ();
    }

    if(currentPageNum != 0)
    {
        prevButton->setIsEnabled (true);
        m_elementlist[currElement]->forceActiveFocus ();
    }
    else
    {
        if((currElement == CAM_SRCH_NEXT_BTN) ||
                (currElement == CAM_SRCH_PREV_BTN))
        {
            currElement = CAM_SRCH_NEXT_BTN;
            m_elementlist[currElement]->forceActiveFocus ();
        }
        prevButton->setIsEnabled (false);
    }

    if(currentPageNum == (maximumPages-1))
    {
        if((currElement == CAM_SRCH_NEXT_BTN) ||
                (currElement == CAM_SRCH_PREV_BTN))
        {
            currElement = CAM_SRCH_PREV_BTN;
            m_elementlist[currElement]->forceActiveFocus ();
        }
        nextButton->setIsEnabled (false);
    }
    else
    {
        nextButton->setIsEnabled (true);
        currElement = CAM_SRCH_NEXT_BTN;
        m_elementlist[currElement]->forceActiveFocus ();
    }
}

void CamSearchFailReport::paintEvent (QPaintEvent *)
{
    QPainter painter(this);
    QColor color;

    color.setAlpha (0);
    painter.setBrush (QBrush(color));
    painter.setPen (Qt::NoPen);

    painter.drawRoundedRect (QRect(0,
                                   0,
                                   SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH),
                                   SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT)),
                             SCALE_WIDTH(RECT_RADIUS), SCALE_HEIGHT(RECT_RADIUS));

    color.setAlpha (150);
    painter.setBrush (QBrush(color));
    painter.setPen (Qt::NoPen);

    painter.drawRoundedRect (QRect(SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH),
                                   SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) -SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT),
                                   SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH),
                                   SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)),
                             SCALE_WIDTH(RECT_RADIUS), SCALE_HEIGHT(RECT_RADIUS));

    if(m_elementlist[currElement]!= NULL)
    {
        m_elementlist[currElement]->forceActiveFocus();
    }
}

void CamSearchFailReport::takeLeftKeyAction()
{
    do
    {
        currElement = (currElement - 1 + MAX_CAM_SRCH_CTRL) % MAX_CAM_SRCH_CTRL;
    }while(!m_elementlist[currElement]->getIsEnabled());

    m_elementlist[currElement]->forceActiveFocus();
}

void CamSearchFailReport::takeRightKeyAction()
{
    do
    {
        currElement = (currElement + 1) % MAX_CAM_SRCH_CTRL;
    }while(!m_elementlist[currElement]->getIsEnabled());

    m_elementlist[currElement]->forceActiveFocus();
}

void CamSearchFailReport::showEvent (QShowEvent *event)
{
    QWidget::showEvent (event);
    if(m_elementlist[currElement] != NULL)
    {
        m_elementlist[currElement]->forceActiveFocus ();
    }
}

void CamSearchFailReport::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void CamSearchFailReport::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    currElement = CAM_SRCH_CLS_BTN;
    m_elementlist[currElement]->forceActiveFocus ();
}

void CamSearchFailReport::slotUpdateCurrentElement (int indexInPage)
{
    currElement = indexInPage;
}

void CamSearchFailReport::slotButtonClick (int indexInPage)
{
    switch(indexInPage)
    {
    case CAM_SRCH_NEXT_BTN:
    {
        if (currentPageNum != (maximumPages-1))
        {
            currentPageNum ++;
        }
        showReports();
    }
        break;

    case CAM_SRCH_PREV_BTN:
    {
        if(currentPageNum > 0)
        {
            currentPageNum --;
        }
        showReports();
    }
        break;
    default:
        emit sigObjectDelete ();
    }
}

void CamSearchFailReport::insertKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void CamSearchFailReport::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void CamSearchFailReport::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}
