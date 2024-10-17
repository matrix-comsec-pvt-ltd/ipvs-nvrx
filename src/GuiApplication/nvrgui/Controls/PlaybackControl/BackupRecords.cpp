#include "BackupRecords.h"
#include <QKeyEvent>
#include <QPainter>

#define BKUP_SINGLE_REC_BGRECT_WIDTH     SCALE_WIDTH(770)
#define BKUP_SINGLE_REC_BGRECT_HEIGHT    SCALE_HEIGHT(580)

#define BKUP_STATUS_BAR_RECT_WIDTH       SCALE_WIDTH(725)
#define BKUP_STATUS_BAR_RECT_HEIGHT      SCALE_HEIGHT(48)

#define BKUP_LEFT_MARGIN                 SCALE_WIDTH(20)

#define BKUP_INFO_STRING                 "Records will be stored in .avi format"
#define BKUP_STATUS_STRING               "Backup Status"

static QString tableHeading[] = { "S.No",
                                  "Start Date Time",
                                  "End Date Time",
                                  "Camera",
                                  "Status" };

const quint8 tableCellWidth[] = {60, 180, 180, 150, 150};

BackupRecords::BackupRecords(QStringList &startDateTimeList,
                             QStringList &endDateTimeList,
                             QStringList &cameraList,
                             QWidget *parent)
    :KeyBoard(parent)
{
    this->setGeometry (0,0, parent->width (), parent->height ());

    m_startDateTimeList = startDateTimeList;
    m_endDateTimeList = endDateTimeList;
    m_cameraList = cameraList;

    createDefaultComponent ();
    setPercentage (0);
    m_statusTextString[0]->changeText ("");
    fillRecords();

    m_currElement = BKUP_REC_CLOSE_BTN;
    m_elementList[m_currElement]->forceActiveFocus ();

    this->show ();
}

BackupRecords::~BackupRecords()
{
    delete m_backGroundRect;
    delete m_BackupLocation;
    delete m_backupStatus;
    delete m_barBackGround;
    delete m_statusRect;
    delete m_percentTextlabel;

    disconnect (m_closeBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    disconnect(m_closeBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete m_closeBtn;

    delete m_heading;
    delete m_infoString;

    disconnect (m_startBtn,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    disconnect(m_startBtn,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpadateCurrentElement(int)));
    delete m_startBtn;

    for(quint8 index = 0; index < MAX_BACKUP_HEADING_FIELDS; index++)
    {
        delete m_headingTextCell[index];
        delete m_headingTextString[index];
    }

    for(quint8 index = 0; index < MAX_BACKUP_LIST_ROW; index++)
    {
        delete m_srNumTextCell[index];
        delete m_srNumTextString[index];
        delete m_startTimeTextCell[index];
        delete m_startTimeTextString[index];
        delete m_endTimeTextCell[index];
        delete m_endTimeTextString[index];
        delete m_cameraNumTextCell[index];
        delete m_cameraNumTextString[index];
        delete m_statusTextCell[index];
        delete m_statusTextString[index];
    }
}

void BackupRecords::createDefaultComponent()
{
    m_currntIndex = 0;
    m_currentPage = 0;

    m_backGroundRect = new Rectangle((this->width () - BKUP_SINGLE_REC_BGRECT_WIDTH)/2,
                                     (SCALE_HEIGHT(20) + (this->height () - BKUP_SINGLE_REC_BGRECT_HEIGHT)/2),
                                     BKUP_SINGLE_REC_BGRECT_WIDTH,
                                     BKUP_SINGLE_REC_BGRECT_HEIGHT,
                                     SCALE_WIDTH(10), BORDER_2_COLOR,
                                     NORMAL_BKG_COLOR, this, 2);

    m_closeBtn = new CloseButtton((m_backGroundRect->x () + m_backGroundRect->width ()),
                                  m_backGroundRect->y (),
                                  SCALE_WIDTH(10),SCALE_HEIGHT(10), this, CLOSE_BTN_TYPE_1,
                                  BKUP_REC_CLOSE_BTN);

    m_elementList[BKUP_REC_CLOSE_BTN] = m_closeBtn;

    connect(m_closeBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect (m_closeBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));

    m_heading = new TextLabel((m_backGroundRect->x () + m_backGroundRect->width ()/2),
                              (m_closeBtn->y () + m_closeBtn->height ()/2),
                              SCALE_FONT(HEADING_FONT_SIZE),
                              "Backup",
                              this,
                              HIGHLITED_FONT_COLOR,
                              NORMAL_FONT_FAMILY,
                              ALIGN_CENTRE_X_CENTER_Y);

    m_BackupLocation = new ReadOnlyElement(m_backGroundRect->x () + BKUP_LEFT_MARGIN  ,
                                           (m_backGroundRect->y () +
                                            m_heading->height () + SCALE_HEIGHT(20)),
                                           (BKUP_SINGLE_REC_BGRECT_WIDTH - SCALE_WIDTH(40)),
                                           BGTILE_HEIGHT,
                                           (SCALE_WIDTH(READONLY_MEDIAM_WIDTH) + SCALE_WIDTH(20)),
                                           READONLY_HEIGHT,
                                           "",
                                           this,
                                           COMMON_LAYER,
                                           SCALE_WIDTH(20), SCALE_WIDTH(10),
                                           "Backup Location");

    m_headingTextCell[0] = new TableCell(m_backGroundRect->x () + BKUP_LEFT_MARGIN  ,
                                         (m_BackupLocation->y () +
                                          m_BackupLocation->height () + SCALE_HEIGHT(10)),
                                         SCALE_WIDTH(tableCellWidth[0]),
                                         TABELCELL_HEIGHT,
                                         this,
                                         true);

    m_headingTextString[0] = new TextLabel(m_headingTextCell[0]->x () + SCALE_WIDTH(10),
                                           (m_headingTextCell[0]->y () +
                                            m_headingTextCell[0]->height ()/2),
                                           NORMAL_FONT_SIZE,
                                           tableHeading[0],
                                           this,
                                           NORMAL_FONT_COLOR,
                                           NORMAL_FONT_FAMILY,
                                           ALIGN_START_X_CENTRE_Y);

    for(quint8 index = 1; index < MAX_BACKUP_HEADING_FIELDS; index++)
    {

        m_headingTextCell[index] = new TableCell((m_headingTextCell[index-1]->x () +
                                                  m_headingTextCell[index-1]->width ()),
                                                 m_headingTextCell[0]->y (),
                                                 SCALE_WIDTH(tableCellWidth[index]),
                                                 TABELCELL_HEIGHT,
                                                 this,
                                                 true);

        m_headingTextString[index] = new TextLabel(m_headingTextCell[index]->x () + SCALE_WIDTH(10),
                                                   (m_headingTextCell[index]->y () +
                                                    m_headingTextCell[index]->height ()/2),
                                                   NORMAL_FONT_SIZE,
                                                   tableHeading[index],
                                                   this,
                                                   NORMAL_FONT_COLOR,
                                                   NORMAL_FONT_FAMILY,
                                                   ALIGN_START_X_CENTRE_Y);
    }

    m_srNumTextCell[0] = new TableCell(m_headingTextCell[0]->x (),
                                       (m_headingTextCell[0]->y () +
                                        m_headingTextCell[0]->height ()),
                                       SCALE_WIDTH(tableCellWidth[0]),
                                       TABELCELL_HEIGHT,
                                       this);

    m_srNumTextString[0] = new TextLabel(m_srNumTextCell[0]->x () + SCALE_WIDTH(10),
                                         (m_srNumTextCell[0]->y () +
                                          m_srNumTextCell[0]->height ()/2),
                                         NORMAL_FONT_SIZE,
                                         "",
                                         this,
                                         NORMAL_FONT_COLOR,
                                         NORMAL_FONT_FAMILY,
                                         ALIGN_START_X_CENTRE_Y);

    m_startTimeTextCell[0] = new TableCell(m_headingTextCell[1]->x (),
                                           (m_headingTextCell[1]->y () +
                                            m_headingTextCell[1]->height ()),
                                           SCALE_WIDTH(tableCellWidth[1]),
                                           TABELCELL_HEIGHT,
                                           this);

    m_startTimeTextString[0] = new TextLabel(m_startTimeTextCell[0]->x () + SCALE_WIDTH(10),
                                             (m_startTimeTextCell[0]->y () +
                                              m_startTimeTextCell[0]->height ()/2),
                                             NORMAL_FONT_SIZE,
                                             "",
                                             this,
                                             NORMAL_FONT_COLOR,
                                             NORMAL_FONT_FAMILY,
                                             ALIGN_START_X_CENTRE_Y);

    m_endTimeTextCell[0] = new TableCell(m_headingTextCell[2]->x (),
                                         (m_headingTextCell[2]->y () +
                                          m_headingTextCell[2]->height ()),
                                         SCALE_WIDTH(tableCellWidth[2]),
                                         TABELCELL_HEIGHT,
                                         this);

    m_endTimeTextString[0] = new TextLabel(m_endTimeTextCell[0]->x () + SCALE_WIDTH(10),
                                           (m_endTimeTextCell[0]->y () +
                                            m_endTimeTextCell[0]->height ()/2),
                                           NORMAL_FONT_SIZE,
                                           "",
                                           this,
                                           NORMAL_FONT_COLOR,
                                           NORMAL_FONT_FAMILY,
                                           ALIGN_START_X_CENTRE_Y);

    m_cameraNumTextCell[0] = new TableCell(m_headingTextCell[3]->x (),
                                           (m_headingTextCell[3]->y () +
                                            m_headingTextCell[3]->height ()),
                                           SCALE_WIDTH(tableCellWidth[3]),
                                           TABELCELL_HEIGHT,
                                           this);

    m_cameraNumTextString[0] = new TextLabel(m_cameraNumTextCell[0]->x () + SCALE_WIDTH(10),
                                             (m_cameraNumTextCell[0]->y () +
                                              m_cameraNumTextCell[0]->height ()/2),
                                             NORMAL_FONT_SIZE,
                                             "",
                                             this,
                                             NORMAL_FONT_COLOR,
                                             NORMAL_FONT_FAMILY,
                                             ALIGN_START_X_CENTRE_Y);

    m_statusTextCell[0] = new TableCell(m_headingTextCell[4]->x (),
                                        (m_headingTextCell[4]->y () +
                                         m_headingTextCell[4]->height ()),
                                        SCALE_WIDTH(tableCellWidth[4]),
                                        TABELCELL_HEIGHT,
                                        this);

    m_statusTextString[0] = new TextLabel(m_statusTextCell[0]->x () + SCALE_WIDTH(10),
                                          (m_statusTextCell[0]->y () +
                                           m_statusTextCell[0]->height ()/2),
                                          NORMAL_FONT_SIZE,
                                          "",
                                          this,
                                          NORMAL_FONT_COLOR,
                                          NORMAL_FONT_FAMILY,
                                          ALIGN_START_X_CENTRE_Y);


    for(quint8 index = 1; index < MAX_BACKUP_LIST_ROW; index++)
    {

        m_srNumTextCell[index] = new TableCell(m_srNumTextCell[(index-1)]->x (),
                                               (m_srNumTextCell[(index-1)]->y () +
                                                m_srNumTextCell[(index-1)]->height ()),
                                               SCALE_WIDTH(tableCellWidth[0]),
                                               TABELCELL_HEIGHT,
                                               this);

        m_srNumTextString[index] = new TextLabel(m_srNumTextCell[index]->x () + SCALE_WIDTH(10),
                                                 (m_srNumTextCell[index]->y () +
                                                  m_srNumTextCell[index]->height ()/2),
                                                 NORMAL_FONT_SIZE,
                                                 "",
                                                 this,
                                                 NORMAL_FONT_COLOR,
                                                 NORMAL_FONT_FAMILY,
                                                 ALIGN_START_X_CENTRE_Y);

        m_startTimeTextCell[index] = new TableCell(m_startTimeTextCell[(index-1)]->x (),
                                                   (m_startTimeTextCell[(index-1)]->y () +
                                                    m_startTimeTextCell[(index-1)]->height ()),
                                                   SCALE_WIDTH(tableCellWidth[1]),
                                                   TABELCELL_HEIGHT,
                                                   this);

        m_startTimeTextString[index] = new TextLabel(m_startTimeTextCell[index]->x () + SCALE_WIDTH(10),
                                                     (m_startTimeTextCell[index]->y () +
                                                      m_startTimeTextCell[index]->height ()/2),
                                                     NORMAL_FONT_SIZE,
                                                     "",
                                                     this,
                                                     NORMAL_FONT_COLOR,
                                                     NORMAL_FONT_FAMILY,
                                                     ALIGN_START_X_CENTRE_Y);

        m_endTimeTextCell[index] = new TableCell(m_endTimeTextCell[(index-1)]->x (),
                                                 (m_endTimeTextCell[(index-1)]->y () +
                                                  m_endTimeTextCell[(index-1)]->height ()),
                                                 SCALE_WIDTH(tableCellWidth[2]),
                                                 TABELCELL_HEIGHT,
                                                 this);

        m_endTimeTextString[index] = new TextLabel(m_endTimeTextCell[index]->x () + SCALE_WIDTH(10),
                                                   (m_endTimeTextCell[index]->y () +
                                                    m_endTimeTextCell[index]->height ()/2),
                                                   NORMAL_FONT_SIZE,
                                                   "",
                                                   this,
                                                   NORMAL_FONT_COLOR,
                                                   NORMAL_FONT_FAMILY,
                                                   ALIGN_START_X_CENTRE_Y);

        m_cameraNumTextCell[index] = new TableCell(m_cameraNumTextCell[(index-1)]->x (),
                                                   (m_cameraNumTextCell[(index-1)]->y () +
                                                    m_cameraNumTextCell[(index-1)]->height ()),
                                                   SCALE_WIDTH(tableCellWidth[3]),
                                                   TABELCELL_HEIGHT,
                                                   this);

        m_cameraNumTextString[index] = new TextLabel(m_cameraNumTextCell[index]->x () + SCALE_WIDTH(10),
                                                     (m_cameraNumTextCell[index]->y () +
                                                      m_cameraNumTextCell[index]->height ()/2),
                                                     NORMAL_FONT_SIZE,
                                                     "",
                                                     this,
                                                     NORMAL_FONT_COLOR,
                                                     NORMAL_FONT_FAMILY,
                                                     ALIGN_START_X_CENTRE_Y);

        m_statusTextCell[index] = new TableCell(m_statusTextCell[(index-1)]->x (),
                                                (m_statusTextCell[(index-1)]->y () +
                                                 m_statusTextCell[(index-1)]->height ()),
                                                SCALE_WIDTH(tableCellWidth[4]),
                                                TABELCELL_HEIGHT,
                                                this);

        m_statusTextString[index] = new TextLabel(m_statusTextCell[index]->x () + SCALE_WIDTH(10),
                                                  (m_statusTextCell[index]->y () +
                                                   m_statusTextCell[index]->height ()/2),
                                                  NORMAL_FONT_SIZE,
                                                  "",
                                                  this,
                                                  NORMAL_FONT_COLOR,
                                                  NORMAL_FONT_FAMILY,
                                                  ALIGN_START_X_CENTRE_Y);
    }

    m_backupStatus = new TextLabel(m_backGroundRect->x () + BKUP_LEFT_MARGIN  ,
                                   m_backGroundRect->y () + m_backGroundRect->height () - SCALE_HEIGHT(140),
                                   NORMAL_FONT_SIZE,
                                   BKUP_STATUS_STRING,
                                   this, HIGHLITED_FONT_COLOR);

    m_barBackGround = new Rectangle((this->width () - BKUP_STATUS_BAR_RECT_WIDTH)/2,
                                    m_backGroundRect->y () + m_backGroundRect->height () - SCALE_HEIGHT(110),
                                    BKUP_STATUS_BAR_RECT_WIDTH,
                                    BKUP_STATUS_BAR_RECT_HEIGHT,
                                    0, BORDER_2_COLOR, NORMAL_BKG_COLOR,
                                    this, 1);

    m_statusRect = new Rectangle(m_barBackGround->x () + (1),
                                 m_barBackGround->y () + (1),
                                 SCALE_WIDTH(50),
                                 (BKUP_STATUS_BAR_RECT_HEIGHT - (2)),
                                 BLUE_COLOR, this);

    m_percentTextlabel = new TextLabel(m_statusRect->x () + (BKUP_STATUS_BAR_RECT_WIDTH/2),
                                       m_statusRect->y () + m_statusRect->height ()/2,
                                       NORMAL_FONT_SIZE,
                                       "",
                                       this,
                                       BORDER_2_COLOR,
                                       NORMAL_FONT_FAMILY,
                                       ALIGN_CENTRE_X_CENTER_Y);

    m_infoString = new TextLabel(m_backGroundRect->x () + BKUP_LEFT_MARGIN  ,
                                 m_backGroundRect->y () + m_backGroundRect->height () - SCALE_HEIGHT(40),
                                 NORMAL_FONT_SIZE,
                                 BKUP_INFO_STRING,
                                 this, NORMAL_FONT_COLOR,
                                 NORMAL_FONT_FAMILY,
                                 ALIGN_START_X_CENTRE_Y);

    m_startBtn = new CnfgButton(CNFGBUTTON_MEDIAM,
                                (m_backGroundRect->x () + m_backGroundRect->width () - SCALE_WIDTH(70)),
                                (m_backGroundRect->y () + m_backGroundRect->height () - SCALE_HEIGHT(35)),
                                "Export",
                                this,
                                BKUP_REC_START_BTN);

    m_elementList[BKUP_REC_START_BTN] = m_startBtn;
    connect(m_startBtn,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpadateCurrentElement(int)));
    connect (m_startBtn,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));
}

void BackupRecords::fillRecords ()
{
    quint8 recordOnPage = 0;
    quint8 eleIndex = 0;

    quint8  maxRecords = m_startDateTimeList.length ();

    m_maxPages = (maxRecords % MAX_BACKUP_LIST_ROW == 0 )?
                (maxRecords / MAX_BACKUP_LIST_ROW) :
                ((maxRecords / MAX_BACKUP_LIST_ROW) + 1);

    if(maxRecords < (MAX_BACKUP_LIST_ROW*(m_currentPage + 1)))
    {
        recordOnPage = maxRecords - ((MAX_BACKUP_LIST_ROW*(m_currentPage)) );
    }
    else
    {
        recordOnPage = MAX_BACKUP_LIST_ROW;
    }

    for(quint8 index = 0; index < recordOnPage; index++)
    {
        eleIndex = ((index + (m_currentPage*MAX_BACKUP_LIST_ROW)));

        m_srNumTextString[index]->changeText (QString("%1").arg (eleIndex + 1));
        m_srNumTextString[index]->update ();

        m_startTimeTextString[index]->changeText (m_startDateTimeList.at (eleIndex));
        m_startTimeTextString[index]->update ();

        m_endTimeTextString[index]->changeText (m_endDateTimeList.at (eleIndex));
        m_endTimeTextString[index]->update ();

        m_cameraNumTextString[index]->changeText (m_cameraList.at (eleIndex));
        m_cameraNumTextString[index]->update ();
    }

    for(quint8 index = recordOnPage; index < MAX_BACKUP_LIST_ROW; index++)
    {
        m_srNumTextString[index]->changeText ("");
        m_srNumTextString[index]->update ();

        m_startTimeTextString[index]->changeText ("");
        m_startTimeTextString[index]->update ();

        m_endTimeTextString[index]->changeText ("");
        m_endTimeTextString[index]->update ();

        m_cameraNumTextString[index]->changeText ("");
        m_cameraNumTextString[index]->update ();
    }
}

void BackupRecords::setPercentage(quint8 percent)
{
    m_statusTextString[m_currntIndex]->changeText ("In Progress");
    m_statusTextString[m_currntIndex]->update ();

    if(percent == 100)
    {
        m_statusTextString[m_currntIndex]->changeText ("Complete");
        m_statusTextString[m_currntIndex]->update ();
        m_currntIndex++;
    }

    m_statusRect->resetGeometry (((BKUP_STATUS_BAR_RECT_WIDTH - 2) * percent *100)/10000);
    m_statusRect->update ();
    m_percentTextlabel->changeText (QString("%1 %").arg (percent));
    m_percentTextlabel->update ();
}

void BackupRecords::updateBackUpLocation(QString str)
{
    m_BackupLocation->changeValue (str);
    m_BackupLocation->update ();
}

void BackupRecords::paintEvent (QPaintEvent *)
{
    QPainter painter(this);
    QColor color;

    color.setAlpha (0);
    painter.setBrush (QBrush(color));
    painter.setPen (Qt::NoPen);
    painter.drawRoundedRect (QRect(0,
                                   0,
                                   PB_SEARCH_HEADER_RECT_WIDTH,
                                   PB_SEARCH_HEADER_RECT_HEIGHT),
                             SCALE_WIDTH(RECT_RADIUS), SCALE_HEIGHT(RECT_RADIUS));

    color.setAlpha (150);
    painter.setBrush (QBrush(color));
    painter.setPen (Qt::NoPen);
    painter.drawRoundedRect (QRect(0,
                                   PB_SEARCH_HEADER_RECT_HEIGHT,
                                   PB_SEARCH_MAIN_RECT_WIDTH,
                                   PB_SEARCH_MAIN_RECT_HEIGHT),
                             SCALE_WIDTH(RECT_RADIUS), SCALE_HEIGHT(RECT_RADIUS));
}

void BackupRecords::takeLeftKeyAction()
{
    do
    {
        m_currElement = (m_currElement - 1 + MAX_BKUP_REC_ELEMETS)
                % MAX_BKUP_REC_ELEMETS;
    }while(!m_elementList[m_currElement]->getIsEnabled());

    m_elementList[m_currElement]->forceActiveFocus();
}

void BackupRecords::takeRightKeyAction()
{
    do
    {
        m_currElement = (m_currElement + 1)
                % MAX_BKUP_REC_ELEMETS;
    }while(!m_elementList[m_currElement]->getIsEnabled());

    m_elementList[m_currElement]->forceActiveFocus();
}

void BackupRecords::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void BackupRecords::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void BackupRecords::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void BackupRecords::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    emit sigBackUpRecsBtnClicked (BKUP_REC_CLOSE_BTN);
}

void BackupRecords::slotUpadateCurrentElement (int index)
{
    m_currElement = index;
}

void BackupRecords::slotButtonClick (int index)
{
    if(index == BKUP_REC_START_BTN)
    {
        m_startBtn->setIsEnabled (false);
    }

    emit sigBackUpRecsBtnClicked (index);
}
