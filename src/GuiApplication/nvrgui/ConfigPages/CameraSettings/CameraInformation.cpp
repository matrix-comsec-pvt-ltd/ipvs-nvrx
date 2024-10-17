#include "CameraInformation.h"
#include <QKeyEvent>
#include <QEvent>

#define CAM_INFO_WIDTH          SCALE_WIDTH(670)
#define CAM_INFO_HEIGHT         SCALE_HEIGHT(520)

static const QString camInfoStrings[] = {
    "Camera List",
    "Previous",
    "Next",
    "Click Set to Replace/add Camera on Particular Index"
};

static const QString camInfoHeadings[] = {
    "Cam No",
    "IP Address",
    "Camera Name",
    "Status",
    "Set"
};

CameraInformation::CameraInformation(QStringList cameraIndex,
                                     QStringList cameraIp,
                                     QStringList cameraName,
                                     QStringList cameraState,
                                     QWidget *parent, quint8 selIndex) :
    KeyBoard(parent)
{
    this->setGeometry (0, 0, parent->width(), parent->height());

    initilizeVariable();

    cameraIndexList = cameraIndex;
    cameraIpList = cameraIp;
    cameraNameList = cameraName;
    cameraStateList = cameraState;
    m_selIndex = selIndex;

    m_maxListCount = cameraIndexList.length ();

    m_maximumPages = (m_maxListCount % MAX_CAMERA_ON_PAGE == 0 )?
                (m_maxListCount / MAX_CAMERA_ON_PAGE) :
                ((m_maxListCount / MAX_CAMERA_ON_PAGE) + 1);

    createDefaultElements();
    showCamInfo();

    m_currElement = CAM_INFO_CLOSE_BUTTON;
    m_elementlist[m_currElement]->forceActiveFocus ();

    this->show ();
}

CameraInformation::~CameraInformation()
{
    delete m_backGround;

    disconnect (m_closeButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    disconnect (m_closeButton,
                SIGNAL(sigUpdateCurrentElement(int)),
                this,
                SLOT(slotUpdateCurrentElement(int)));
    delete m_closeButton;
    delete m_heading;
    delete m_caminfoElideStr;
    delete elementHeading;

    for(quint8 index = 0; index < MAX_INFO_FIELDS; index++)
    {
        delete m_caminfoHeading[index];
        delete m_caminfoHeadingStr[index];
    }

    for(quint8 index = 0 ; index < MAX_CAMERA_ON_PAGE; index++)
    {
        delete m_caminfoSrNum[index];
        delete m_caminfoSrNumStr[index];

        delete m_caminfoIp[index];
        delete m_caminfoIpStr[index];

        delete m_caminfoCamName[index];
        delete m_caminfoCamNameStr[index];

        delete m_caminfoCamState[index];
        delete m_caminfoCamStateStr[index];

        delete m_caminfoSet[index];

        disconnect (m_caminfoSetButton[index],
                    SIGNAL(sigButtonClick(int)),
                    this,
                    SLOT(slotButtonClick(int)));

        disconnect (m_caminfoSetButton[index],
                    SIGNAL(sigUpdateCurrentElement(int)),
                    this,
                    SLOT(slotUpdateCurrentElement(int)));
        delete m_caminfoSetButton[index];
    }

    disconnect(m_previousButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));

    disconnect (m_previousButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    delete m_previousButton;

    delete m_caminfoBottomString;

    disconnect(m_nextButton,
               SIGNAL(sigUpdateCurrentElement(int)),
               this,
               SLOT(slotUpdateCurrentElement(int)));

    disconnect (m_nextButton,
                SIGNAL(sigButtonClick(int)),
                this,
                SLOT(slotButtonClick(int)));
    delete m_nextButton;

    cameraIndexList.clear ();
    cameraIpList.clear ();
    cameraNameList.clear ();
    cameraStateList.clear ();
}

void CameraInformation::initilizeVariable ()
{
    m_backGround = NULL;
    m_heading = NULL;
    m_closeButton = NULL;

    for(quint8 index = 0; index < MAX_INFO_FIELDS; index++)
    {
        m_caminfoHeading[index] = NULL;
        m_caminfoHeadingStr[index] = NULL;
    }

    m_caminfoElideStr = NULL;
    for(quint8 index = 0 ; index < MAX_CAMERA_ON_PAGE; index++)
    {
        m_caminfoSrNum[index] = NULL;
        m_caminfoSrNumStr[index] = NULL;

        m_caminfoIp[index] = NULL;
        m_caminfoIpStr[index] = NULL;

        m_caminfoCamName[index] = NULL;
        m_caminfoCamNameStr[index] = NULL;

        m_caminfoCamState[index] = NULL;
        m_caminfoCamStateStr[index] = NULL;

        m_caminfoSet[index] = NULL;
        m_caminfoSetButton[index] = NULL;
    }

    m_previousButton = NULL;

    m_caminfoBottomString = NULL;

    m_nextButton =  NULL;

    cameraIndexList.clear ();
    cameraIpList.clear ();
    cameraNameList.clear ();
    cameraStateList.clear ();
    m_currentPageNo = 0;
}

void CameraInformation::createDefaultElements ()
{
    quint8 headerWidthArray[] = {55, 150, 250, 100, 70};

    m_backGround = new Rectangle((SCALE_WIDTH(SETTING_LEFT_PANEL_WIDTH) + SCALE_WIDTH(20) + ((SCALE_WIDTH(SETTING_RIGHT_PANEL_WIDTH) - CAM_INFO_WIDTH) / 2)),
                                 (SCALE_HEIGHT(SETTING_LEFT_PANEL_HEIGHT) - SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT) + ((SCALE_HEIGHT(SETTING_RIGHT_PANEL_HEIGHT)- CAM_INFO_HEIGHT) / 2)),
                                 CAM_INFO_WIDTH,
                                 CAM_INFO_HEIGHT,
                                 0,
                                 NORMAL_BKG_COLOR,
                                 NORMAL_BKG_COLOR,
                                 this);

    if(m_backGround != NULL)
    {
        m_closeButton = new CloseButtton ((m_backGround->x ()+ m_backGround->width () - SCALE_WIDTH(20)),
                                          (m_backGround->y () + SCALE_HEIGHT(20)),
                                          this,
                                          CLOSE_BTN_TYPE_1,
                                          CAM_INFO_CLOSE_BUTTON);

        m_elementlist[CAM_INFO_CLOSE_BUTTON] = m_closeButton;

        connect (m_closeButton,
                 SIGNAL(sigButtonClick(int)),
                 this,
                 SLOT(slotButtonClick(int)));

        connect (m_closeButton,
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));


        m_heading = new Heading((m_backGround->x () + (m_backGround->width () / 2)),
                                (m_backGround->y () + SCALE_HEIGHT(30)),
                                camInfoStrings[0],
                                this,
                                HEADING_TYPE_2);

        m_caminfoHeading[0] = new TableCell (m_backGround->x () + SCALE_WIDTH(20),
                                             m_backGround->y () + SCALE_HEIGHT(65),
                                             SCALE_WIDTH(headerWidthArray[0]),
                                             SCALE_HEIGHT(50),
                                             this,
                                             true);
        m_caminfoHeadingStr[0] = NULL;

        elementHeading = new ElementHeading(m_caminfoHeading[0]->x () + SCALE_WIDTH(10),
                                                    m_caminfoHeading[0]->y () + SCALE_HEIGHT(6),
                                                    (SCALE_WIDTH(headerWidthArray[0]) - SCALE_WIDTH(10) ),
                                                    SCALE_HEIGHT(50),
                                                   "",
                                                    NO_LAYER,
                                                    this,
                                                    false,
                                                    0,SCALE_WIDTH(12));

        QString fontColor = "#c8c8c8";
        QString fontWidth = "" + QString::number(SCALE_WIDTH(15)) +"px";
        QString styl = "ElidedLabel \
        { \
            color: %1; \
            font-size: %2; \
            font-family: %3; \
        }";

        elementHeading->setStyleSheet(styl.arg(fontColor).arg(fontWidth).arg(NORMAL_FONT_FAMILY));
        m_caminfoElideStr = new ElidedLabel(Multilang(camInfoHeadings[0].toUtf8().constData()), elementHeading);
        m_caminfoElideStr->resize(SCALE_WIDTH(headerWidthArray[0]) - 5, SCALE_HEIGHT(50));
        m_caminfoElideStr->show();


        for(quint8 index = 1; index < MAX_INFO_FIELDS; index++)
        {
            m_caminfoHeading[index] = new TableCell (m_caminfoHeading[index -1]->x () +
                                                     m_caminfoHeading[index -1]->width (),
                                                     m_caminfoHeading[index -1]->y (),
                                                     SCALE_WIDTH(headerWidthArray[index]) - 1,
                                                     SCALE_HEIGHT(50),
                                                     this,
                                                     true);


            m_caminfoHeadingStr[index] = new TextLabel(m_caminfoHeading[(index)]->x () +
                                                       (m_caminfoHeading[(index)]->width ())/2,
                                                       m_caminfoHeading[(index)]->y () +
                                                       (m_caminfoHeading[index]->height ())/2,
                                                       NORMAL_FONT_SIZE,
                                                       camInfoHeadings[index],
                                                       this,
                                                       NORMAL_FONT_COLOR,
                                                       NORMAL_FONT_FAMILY,
                                                       ALIGN_CENTRE_X_CENTER_Y, 0, 0, SCALE_WIDTH(headerWidthArray[index]) - 1);
        }

        m_caminfoSrNum[0] = new TableCell(m_caminfoHeading[0]->x (),
                                          m_caminfoHeading[0]->y () +
                                          m_caminfoHeading[0]->height (),
                                          (m_caminfoHeading[0]->width () - 1),
                                          BGTILE_HEIGHT,
                                          this);

        m_caminfoSrNumStr[0] = new TextLabel(m_caminfoSrNum[0]->x () + SCALE_WIDTH(10),
                                             m_caminfoSrNum[0]->y () +
                                             (m_caminfoSrNum[0]->height ())/2,
                                             NORMAL_FONT_SIZE,
                                             "",
                                             this,
                                             NORMAL_FONT_COLOR,
                                             NORMAL_FONT_FAMILY,
                                             ALIGN_START_X_CENTRE_Y, 0, 0, (m_caminfoHeading[0]->width () - 1));

        m_caminfoIp[0] = new TableCell(m_caminfoHeading[1]->x (),
                                       m_caminfoHeading[1]->y () +
                                       m_caminfoHeading[1]->height (),
                                       m_caminfoHeading[1]->width (),
                                       BGTILE_HEIGHT,
                                       this);

        m_caminfoIpStr[0] = new TextLabel(m_caminfoIp[0]->x () + SCALE_WIDTH(10),
                                          m_caminfoIp[0]->y () +
                                          (m_caminfoIp[0]->height ())/2,
                                          NORMAL_FONT_SIZE,
                                          "",
                                          this,
                                          NORMAL_FONT_COLOR,
                                          NORMAL_FONT_FAMILY,
                                          ALIGN_START_X_CENTRE_Y, 0, 0, m_caminfoHeading[1]->width ());

        m_caminfoCamName[0] = new TableCell(m_caminfoHeading[2]->x (),
                                            m_caminfoHeading[2]->y () +
                                            m_caminfoHeading[2]->height (),
                                            (m_caminfoHeading[2]->width () -1),
                                            BGTILE_HEIGHT,
                                            this);

        m_caminfoCamNameStr[0] = new TextLabel(m_caminfoCamName[0]->x () + SCALE_WIDTH(10),
                                               m_caminfoCamName[0]->y () +
                                               (m_caminfoCamName[0]->height ())/2,
                                               NORMAL_FONT_SIZE,
                                               "",
                                               this,
                                               NORMAL_FONT_COLOR,
                                               NORMAL_FONT_FAMILY,
                                               ALIGN_START_X_CENTRE_Y, 0, 0, m_caminfoHeading[2]->width ());

        m_caminfoCamName[0] = new TableCell(m_caminfoHeading[2]->x (),
                                            m_caminfoHeading[2]->y () +
                                            m_caminfoHeading[2]->height (),
                                            (m_caminfoHeading[2]->width () -1),
                                            BGTILE_HEIGHT,
                                            this);

        m_caminfoCamNameStr[0] = new TextLabel(m_caminfoCamName[0]->x () + SCALE_WIDTH(10),
                                               m_caminfoCamName[0]->y () +
                                               (m_caminfoCamName[0]->height ())/2,
                                               NORMAL_FONT_SIZE,
                                               "",
                                               this,
                                               NORMAL_FONT_COLOR,
                                               NORMAL_FONT_FAMILY,
                                               ALIGN_START_X_CENTRE_Y, 0, 0, m_caminfoHeading[2]->width ());

        m_caminfoCamState[0] = new TableCell(m_caminfoHeading[3]->x (),
                                             m_caminfoHeading[3]->y () +
                                             m_caminfoHeading[3]->height (),
                                             (m_caminfoHeading[3]->width () -1),
                                             BGTILE_HEIGHT,
                                             this);

        m_caminfoCamStateStr[0] = new TextLabel(m_caminfoCamState[0]->x () + SCALE_WIDTH(10),
                                                m_caminfoCamState[0]->y () +
                                                (m_caminfoCamState[0]->height ())/2,
                                                NORMAL_FONT_SIZE,
                                                "",
                                                this,
                                                NORMAL_FONT_COLOR,
                                                NORMAL_FONT_FAMILY,
                                                ALIGN_START_X_CENTRE_Y, 0, 0, m_caminfoHeading[3]->width ());

        m_caminfoSet[0] = new TableCell(m_caminfoHeading[4]->x (),
                                        m_caminfoHeading[4]->y () +
                                        m_caminfoHeading[4]->height (),
                                        (m_caminfoHeading[4]->width () - 1),
                                        BGTILE_HEIGHT,
                                        this);


        m_caminfoSetButton[0] = new ControlButton(SET_BUTTON_INDEX,
                                                  m_caminfoSet[0]->x () + SCALE_WIDTH(20),
                                                  m_caminfoSet[0]->y (),
                                                  m_caminfoSet[0]->width (),
                                                  m_caminfoSet[0]->height (),
                                                  this,
                                                  NO_LAYER,
                                                  -1,
                                                  "",
                                                  true,
                                                  CAM_INFO_SET_BUTTON);

        m_elementlist[CAM_INFO_SET_BUTTON] = m_caminfoSetButton[0];

        connect (m_caminfoSetButton[0],
                 SIGNAL(sigButtonClick(int)),
                 this,
                 SLOT(slotButtonClick(int)));

        connect (m_caminfoSetButton[0],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));

    }

    for(quint8 index = 1 ; index < MAX_CAMERA_ON_PAGE; index++)
    {

        m_caminfoSrNum[index] = new TableCell(m_caminfoSrNum[(index-1)]->x (),
                                              m_caminfoSrNum[(index-1)]->y () +
                                              m_caminfoSrNum[(index-1)]->height (),
                                              (m_caminfoSrNum[(index-1)]->width () - 1),
                                              BGTILE_HEIGHT,
                                              this);

        m_caminfoSrNumStr[index] = new TextLabel(m_caminfoSrNum[(index)]->x () + SCALE_WIDTH(10),
                                                 m_caminfoSrNum[(index)]->y () +
                                                 (m_caminfoSrNum[(index)]->height ())/2,
                                                 NORMAL_FONT_SIZE,
                                                 "",
                                                 this,
                                                 NORMAL_FONT_COLOR,
                                                 NORMAL_FONT_FAMILY,
                                                 ALIGN_START_X_CENTRE_Y, 0, 0, (m_caminfoSrNum[(index-1)]->width () - 1));

        m_caminfoIp[(index)] = new TableCell(m_caminfoIp[(index-1)]->x (),
                                             m_caminfoIp[(index-1)]->y () +
                                             m_caminfoIp[(index-1)]->height (),
                                             (m_caminfoIp[(index-1)]->width () - 1),
                                             BGTILE_HEIGHT,
                                             this);

        m_caminfoIpStr[(index)] = new TextLabel(m_caminfoIp[(index)]->x () + SCALE_WIDTH(10),
                                                m_caminfoIp[(index)]->y () +
                                                (m_caminfoIp[(index)]->height ())/2,
                                                NORMAL_FONT_SIZE,
                                                "",
                                                this,
                                                NORMAL_FONT_COLOR,
                                                NORMAL_FONT_FAMILY,
                                                ALIGN_START_X_CENTRE_Y, 0, 0, (m_caminfoIp[(index-1)]->width () - 1));

        m_caminfoCamName[(index)] = new TableCell(m_caminfoCamName[(index-1)]->x (),
                                                  m_caminfoCamName[(index-1)]->y () +
                                                  m_caminfoCamName[(index-1)]->height (),
                                                  (m_caminfoCamName[(index-1)]->width () - 1),
                                                  BGTILE_HEIGHT,
                                                  this);

        m_caminfoCamNameStr[(index)] = new TextLabel(m_caminfoCamName[(index)]->x () + SCALE_WIDTH(10),
                                                     m_caminfoCamName[(index)]->y () +
                                                     (m_caminfoCamName[(index)]->height ())/2,
                                                     NORMAL_FONT_SIZE,
                                                     "",
                                                     this,
                                                     NORMAL_FONT_COLOR,
                                                     NORMAL_FONT_FAMILY,
                                                     ALIGN_START_X_CENTRE_Y, 0, 0, (m_caminfoCamName[(index-1)]->width () - 1));

        m_caminfoCamState[(index)] = new TableCell(m_caminfoCamState[(index-1)]->x (),
                                                   m_caminfoCamState[(index-1)]->y () +
                                                   m_caminfoCamState[(index-1)]->height (),
                                                   (m_caminfoCamState[(index-1)]->width () - 1),
                                                   BGTILE_HEIGHT,
                                                   this);

        m_caminfoCamStateStr[(index)] = new TextLabel(m_caminfoCamState[(index)]->x () + SCALE_WIDTH(10),
                                                      m_caminfoCamState[(index)]->y () +
                                                      (m_caminfoCamState[(index)]->height ())/2,
                                                      NORMAL_FONT_SIZE,
                                                      "",
                                                      this,
                                                      NORMAL_FONT_COLOR,
                                                      NORMAL_FONT_FAMILY,
                                                      ALIGN_START_X_CENTRE_Y, 0, 0, (m_caminfoCamState[(index-1)]->width () - 1));

        m_caminfoSet[(index)] = new TableCell(m_caminfoSet[(index-1)]->x (),
                                              m_caminfoSet[(index-1)]->y () +
                                              m_caminfoSet[(index-1)]->height (),
                                              (m_caminfoSet[(index-1)]->width () - 1),
                                              BGTILE_HEIGHT,
                                              this);


        m_caminfoSetButton[(index)] = new ControlButton(SET_BUTTON_INDEX,
                                                        m_caminfoSet[(index)]->x () + SCALE_WIDTH(20),
                                                        m_caminfoSet[(index)]->y (),
                                                        m_caminfoSet[(index)]->width (),
                                                        m_caminfoSet[(index)]->height (),
                                                        this,
                                                        NO_LAYER,
                                                        -1,
                                                        "",
                                                        true,
                                                        (CAM_INFO_SET_BUTTON + index));

        m_elementlist[CAM_INFO_SET_BUTTON + index] = m_caminfoSetButton[index];

        connect (m_caminfoSetButton[(index)],
                 SIGNAL(sigButtonClick(int)),
                 this,
                 SLOT(slotButtonClick(int)));

        connect (m_caminfoSetButton[(index)],
                 SIGNAL(sigUpdateCurrentElement(int)),
                 this,
                 SLOT(slotUpdateCurrentElement(int)));
    }

    m_previousButton = new ControlButton(PREVIOUS_BUTTON_INDEX,
                                         m_caminfoHeading[0]->x (),
                                         m_caminfoSet[(MAX_CAMERA_ON_PAGE - 1)]->y () +
                                         m_caminfoSet[(MAX_CAMERA_ON_PAGE - 1)]->height (),
                                         m_backGround->width () - SCALE_WIDTH(40),
                                         BGTILE_HEIGHT,
                                         this,
                                         NO_LAYER,
                                         10,
                                         camInfoStrings[1],
                                         false,
                                         CAM_INFO_PREVIOUS_BUTTON,
                                         false);

    m_elementlist[CAM_INFO_PREVIOUS_BUTTON] = m_previousButton;

    connect(m_previousButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    connect (m_previousButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));


    m_caminfoBottomString = new TextLabel((m_backGround->x () + m_backGround->width ()/2),
                                          (m_previousButton->y () + m_previousButton->height () + SCALE_HEIGHT(10)) ,
                                          NORMAL_FONT_SIZE,
                                          camInfoStrings[3],
                                          this,
                                          NORMAL_FONT_COLOR,
                                          NORMAL_FONT_FAMILY,
                                          ALIGN_CENTRE_X_START_Y, 0, 0, CAM_INFO_WIDTH);

    m_nextButton = new ControlButton(NEXT_BUTTON_INDEX,
                                     m_previousButton->x () + m_backGround->width () - SCALE_WIDTH(120),
                                     m_previousButton->y (),
                                     m_previousButton->width (),
                                     BGTILE_HEIGHT,
                                     this,
                                     NO_LAYER,
                                     -1,
                                     camInfoStrings[2],
                                     true,
                                     CAM_INFO_NEXT_BUTTON);

    m_elementlist[CAM_INFO_NEXT_BUTTON] = m_nextButton;

    connect(m_nextButton,
            SIGNAL(sigUpdateCurrentElement(int)),
            this,
            SLOT(slotUpdateCurrentElement(int)));

    connect (m_nextButton,
             SIGNAL(sigButtonClick(int)),
             this,
             SLOT(slotButtonClick(int)));
}

void CameraInformation::showCamInfo ()
{
    quint8 recordOnPage;

    updateNavigationControlStatus();

    if(m_maxListCount < (MAX_CAMERA_ON_PAGE*(m_currentPageNo + 1)))
    {
        recordOnPage = m_maxListCount - ((MAX_CAMERA_ON_PAGE*(m_currentPageNo)) );
    }
    else
    {
        recordOnPage = MAX_CAMERA_ON_PAGE;
    }

    for(quint8 index = 0; index < recordOnPage; index++)
    {
        m_caminfoSrNumStr[index]->changeText (cameraIndexList.at (index + (MAX_CAMERA_ON_PAGE*m_currentPageNo)));
        m_caminfoSrNumStr[index]->update ();

        m_caminfoIpStr[index]->changeText (cameraIpList.at (index + (MAX_CAMERA_ON_PAGE*m_currentPageNo)));
        m_caminfoIpStr[index]->update ();

        m_caminfoCamNameStr[index]->changeText (cameraNameList.at (index + (MAX_CAMERA_ON_PAGE*m_currentPageNo)));
        m_caminfoCamNameStr[index]->update ();

        m_caminfoCamStateStr[index]->changeText (cameraStateList.at (index + (MAX_CAMERA_ON_PAGE*m_currentPageNo)));
        m_caminfoCamStateStr[index]->update ();
        m_caminfoSetButton[index]->setVisible (true);
    }

    for(quint8 index = recordOnPage; index < MAX_CAMERA_ON_PAGE; index++)
    {
        m_caminfoSrNumStr[index]->changeText ("");
        m_caminfoSrNumStr[index]->update ();

        m_caminfoIpStr[index]->changeText ("");
        m_caminfoIpStr[index]->update ();

        m_caminfoCamNameStr[index]->changeText ("");
        m_caminfoCamNameStr[index]->update ();

        m_caminfoCamStateStr[index]->changeText ("");
        m_caminfoCamStateStr[index]->update ();

        m_caminfoSetButton[index]->setVisible (false);
    }
}

void CameraInformation::updateNavigationControlStatus ()
{
    if(m_maxListCount <= MAX_CAMERA_ON_PAGE)
    {
        m_previousButton->setVisible (false);
        m_nextButton->setVisible (false);
    }
    else
    {
        m_previousButton->setIsEnabled (false);
        m_nextButton->setIsEnabled (false);

        m_previousButton->setIsEnabled ((m_currentPageNo != 0 ? true : false ));

        if( m_currentPageNo < (m_maximumPages - 1 ) )
        {
            m_nextButton->setIsEnabled (true);
            if((m_currElement != CAM_INFO_PREVIOUS_BUTTON) || (m_currentPageNo == 0))
                m_currElement = CAM_INFO_NEXT_BUTTON;
        }
        else if( m_currentPageNo == (m_maximumPages - 1 ))
        {
            m_nextButton->setIsEnabled (false);
            m_currElement = CAM_INFO_PREVIOUS_BUTTON;
        }
        m_elementlist[m_currElement]->forceActiveFocus ();
    }
}

void CameraInformation::takeLeftKeyAction()
{
    do
    {
        m_currElement = (m_currElement - 1 + MAX_CAM_INFO_CTRL) % MAX_CAM_INFO_CTRL;
    }while(!m_elementlist[m_currElement]->getIsEnabled());

    m_elementlist[m_currElement]->forceActiveFocus();
}

void CameraInformation::takeRightKeyAction()
{
    do
    {
        m_currElement = (m_currElement + 1) % MAX_CAM_INFO_CTRL;
    }while(!m_elementlist[m_currElement]->getIsEnabled());

    m_elementlist[m_currElement]->forceActiveFocus();
}

void CameraInformation::showEvent (QShowEvent *event)
{
    QWidget::showEvent (event);
    if(m_elementlist[m_currElement] != NULL)
    {
        m_elementlist[m_currElement]->forceActiveFocus ();
    }
}

void CameraInformation::navigationKeyPressed(QKeyEvent *event)
{
    event->accept();
}

void CameraInformation::escKeyPressed(QKeyEvent *event)
{
    event->accept();
    m_currElement = CAM_INFO_CLOSE_BUTTON;
    m_elementlist[m_currElement]->forceActiveFocus ();
}

void CameraInformation::tabKeyPressed(QKeyEvent *event)
{
    event->accept();
    takeRightKeyAction();
}

void CameraInformation::backTab_KeyPressed(QKeyEvent *event)
{
    event->accept();
    takeLeftKeyAction();
}

void CameraInformation::slotButtonClick(int index)
{
    switch(index)
    {
    case CAM_INFO_PREVIOUS_BUTTON:
        if(m_currentPageNo > 0)
        {
            m_currentPageNo --;
        }
        showCamInfo ();
        break;

    case CAM_INFO_NEXT_BUTTON:
        if (m_currentPageNo != (m_maximumPages - 1))
        {
            m_currentPageNo ++;
        }
        showCamInfo ();
        break;

    case CAM_INFO_CLOSE_BUTTON:
        emit sigObjectDelete(m_selIndex);
        break;

    default:
        m_selIndex = m_caminfoSrNumStr[(index - CAM_INFO_SET_BUTTON)]->getText ().toUInt ();
        emit sigObjectDelete(m_selIndex);

    }
}

void CameraInformation::slotUpdateCurrentElement (int index)
{
    m_currElement = index;
}
